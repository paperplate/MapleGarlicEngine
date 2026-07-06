/*
 * The idea is for this module to run at program start, store all assets/files/etc that
 * need to be loaded in a map or list of constexpr variables. Verify it can find all of them,
 * then pass the paths to the various components, sprites, textures, etc. Then I don't need to
 * handle missing files individually
 */
