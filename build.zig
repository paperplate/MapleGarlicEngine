const std = @import("std");
const zcc = @import("compile_commands");

const CPP_ROOT_DIR = "./src/";
const CPP_FLAGS = &.{"-Wall", "-std=c++23", "-fmodule-file=MapleGarlicEngine=MapleGarlicEngine.pcm" };

const HOMEBREW_DIR = "~/../linuxbrew/.linuxbrew";


fn appendFlags(base: []const []const u8, extra: []const u8, allocator: std.mem.Allocator) ![]const []const u8 {
    var list = std.ArrayList([]const u8).init(allocator);
    try list.appendSlice(base);
    try list.append(extra);
    const owned = try list.toOwnedSlice();
    return @as([]const []const u8, owned);
}

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    const version = std.SemanticVersion{ .major = 0, .minor = 0, .patch = 0 };
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    var targets = std.ArrayList(*std.Build.Step.Compile).init(b.allocator);

    const linkage = b.option(std.builtin.LinkMode, "linkage", "Link mode for MapleGarlicEngine") orelse .static;

    const modMapleGarlicEngine = b.createModule(.{
        .target = target,
        .optimize = optimize,
    });

    //const engineFlags = try appendFlags(CPP_FLAGS, "-MJ=MapleGarlicEngine.o.json", allocator);
    //defer allocator.free(engineFlags);

    modMapleGarlicEngine.addCSourceFiles(.{
        .root = b.path(CPP_ROOT_DIR),
        .files = &.{"helloTriangle.cxx", "helloTriangle.hxx"},
        .flags = &.{
            "-Wall",
            "-std=c++23",
            "-fmodule-file=MapleGarlicEngine=MapleGarlicEngine.pcm",
            },//engineFlags,//&.{
    //        "-Wall",
  //          "-std=c++23",
//        }, //"--precompile", "-fprebuilt-module-path=.", "-o MapleGarlicEngine.pcm" },
    });

    const libMapleGarlicEngine = b.addLibrary(.{
        .linkage = linkage,
        .name = "MapleGarlicEngine",
        .version = version,
        .root_module = modMapleGarlicEngine,
    });

    targets.append(libMapleGarlicEngine) catch @panic("OOM");

    // include libraries installed via homebrew
    libMapleGarlicEngine.addIncludePath(b.path("../../../home/linuxbrew/.linuxbrew/include"));
    //libMapleGarlicEngine.addIncludePath(b.path(HOMEBREW_DIR ++ "/include"));
    //libMapleGarlicEngine.addLibraryPath(b.path(HOMEBREW_DIR ++ "/lib"));
    libMapleGarlicEngine.addLibraryPath(b.path("../../../home/linuxbrew/.linuxbrew/lib"));

    libMapleGarlicEngine.addIncludePath(b.path("./thirdparty"));

    libMapleGarlicEngine.linkSystemLibrary("vulkan");
    libMapleGarlicEngine.linkSystemLibrary("glfw3");
    libMapleGarlicEngine.linkLibCpp();

    // This declares intent for the library to be installed into the standard
    // location when the user invokes the "install" step (the default step when
    // running `zig build`).
    b.installArtifact(libMapleGarlicEngine);

    const exeMod = b.createModule(.{
        .target = target,
        .optimize = optimize,
    });

    exeMod.addImport("MapleGarlicEngine", modMapleGarlicEngine);
    //exeMod.linkLibrary(libMapleGarlicEngine);
    exeMod.addIncludePath(b.path("./src"));
    
    //const exeFlags = try appendFlags(CPP_FLAGS, "-MJ 'exe.o.json'", allocator);
    //defer allocator.free(exeFlags);

    exeMod.addCSourceFiles(.{
        .root = b.path(CPP_ROOT_DIR),
        .files = &.{"main.cxx"},
        .flags = &.{"-Wall", "-std=c++23",},//exeFlags,
            //"-Wall",
          //  "-std=c++23",
        //}, // "-fmodule-file=MapleGarlicEngine=MapleGarlicEngine.pcm" },
    });

    exeMod.addIncludePath(b.path("../../../home/linuxbrew/.linuxbrew/include"));
    //exeMod.addIncludePath(b.path(HOMEBREW_DIR ++ "/include"));
    //exeMod.addLibraryPath(b.path(HOMEBREW_DIR ++ "/lib"));
    exeMod.addLibraryPath(b.path("../../../home/linuxbrew/.linuxbrew/lib"));
    exeMod.addIncludePath(b.path("./thirdparty"));

    const exe = b.addExecutable(.{
        .name = "VulkanTest",
        .root_module = exeMod,
    });
    exe.linkLibCpp();

    targets.append(exe) catch @panic("OOM");
    //compile_all_shaders(b, exe);
    //
    zcc.createStep(b, "cdb", targets.toOwnedSlice() catch @panic("OOM"));

    //exe.linkLibrary(libMapleGarlicEngine);
    // This declares intent for the executable to be installed into the
    // standard location when the user invokes the "install" step (the default
    // step when running `zig build`).
    b.installArtifact(exe);

    //const installBin = b.addInstallBinFile(HOMEBREW_DIR ++ "/lib/libglfw3.so", "./libglfw3.so.0");
    //b.installFile(HOMEBREW_DIR ++ "/lib/libglfw3.so", "./libglfw3.so.0");
    //b.getInstallStep().dependOn(&installBin.step);

    // This *creates* a Run step in the build graph, to be executed when another
    // step is evaluated that depends on it. The next line below will establish
    // such a dependency.
    const run_cmd = b.addRunArtifact(exe);

    // By making the run step depend on the install step, it will be run from the
    // installation directory rather than directly from within the cache directory.
    // This is not necessary, however, if the application depends on other installed
    // files, this ensures they will be present and in the expected location.
    run_cmd.step.dependOn(b.getInstallStep());

    // This allows the user to pass arguments to the application in the build
    // command itself, like this: `zig build run -- arg1 arg2 etc`
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    // This creates a build step. It will be visible in the `zig build --help` menu,
    // and can be selected like this: `zig build run`
    // This will evaluate the `run` step rather than the default, which is "install".
    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    // Creates a step for unit testing. This only builds the test executable
    // but does not run it.
    //const lib_unit_tests = b.addTest(.{
    //    .root_source_file = b.path("src/root.zig"),
    //    .target = target,
    //    .optimize = optimize,
    //});

    //const run_lib_unit_tests = b.addRunArtifact(lib_unit_tests);

    //const exe_unit_tests = b.addTest(.{
    //    .root_source_file = b.path("src/main.zig"),
    //    .target = target,
    //    .optimize = optimize,
    //});

    //const run_exe_unit_tests = b.addRunArtifact(exe_unit_tests);

    // Similar to creating the run step earlier, this exposes a `test` step to
    // the `zig build --help` menu, providing a way for the user to request
    // running the unit tests.
    //const test_step = b.step("test", "Run unit tests");
    //test_step.dependOn(&run_lib_unit_tests.step);
    //test_step.dependOn(&run_exe_unit_tests.step);
}

// https://github.com/spanzeri/vkguide-zig/blob/main/build.zig
fn compile_all_shaders(b: *std.Build, exe: *std.Build.Step.Compile) void {

    const shaders_dir = if (@hasDecl(@TypeOf(b.build_root.handle), "openIterableDir"))
        b.build_root.handle.openIterableDir("shaders", .{}) catch @panic("Failed to open shaders directory")
    else
        b.build_root.handle.openDir("shaders", .{ .iterate = true }) catch @panic("Failed to open shaders directory");

    var file_it = shaders_dir.iterate();
    while (file_it.next() catch @panic("Failed to iterate shader directory")) |entry| {
        if (entry.kind == .file) {
            const ext = std.fs.path.extension(entry.name);
            if (std.mem.eql(u8, ext, ".glsl")) {
                const basename = std.fs.path.basename(entry.name);
                const name = basename[0..basename.len - ext.len];

                std.debug.print("Found shader file to compile: {s}. Compiling with name: {s}\n", .{entry.name, name});
                add_shader(b, exe, name);
            }
        }
    }
}

fn add_shader(b: *std.Build, exe: *std.Build.Step.Compile, name: []const u8) void {
    const source = std.fmt.allocPrint(b.allocator, "shaders/{s}.glsl", .{name}) catch @panic("OOM");
    const outpath = std.fmt.allocPrint(b.allocator, "shaders/{s}.spv", .{name}) catch @panic("OOM");

    const shader_compilation = b.addSystemCommand(&.{"glslangValidator"});
    shader_compilation.addArg("-V");
    shader_compilation.addFileArg(b.path(source));
    shader_compilation.addArg("-o");
    const output = shader_compilation.addOutputFileArg(outpath);

    //b.getInstallStep().dependOn(&b.addInstallFileWithDir(output, .prefix
    //const ou = shader_compilation.captureStdOut();
    std.log.info("{s}\n", .{shader_compilation});

    exe.root_module.addAnonymousImport(name, .{.root_source_file = output});

    //b.getInstallStep().dependOn(&b.addInstallFileWithDir(output, .{.custom = "./shaders"}, name).step);
}
