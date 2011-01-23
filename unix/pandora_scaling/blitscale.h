
#ifndef h_blitscale_h
#define h_blitscale_h

typedef enum {
  bs_error,             // fail!
  bs_1to1,              // identity 1:1
  bs_1to2_double,       // 1:2 doubling (no AA/etc)
  bs_1to2_scale2x,      // 1:2 doubling filter
  bs_1to2_smooth,       // 1:2 with smoothing
  bs_1to32_multiplied,  // 1:(3x2) 'doubled' (no AA,etc)
  bs_1to32_smooth,      // 1:(3x2) smoothed
  bs_fs_aspect_multiplied, // scale fullscreen, limited to aspect correct, unsmoothed
  bs_fs_aspect_smooth,     // scale fullscreen, limited to aspect correct, smoothed
  bs_fs_always_multiplied, // arbitrary scale fullscreen, unsmoothed
  bs_fs_always_smooth,     // arbitrary scale fullscreen, smoothed
  bs_max
} blit_scaler_e;

typedef struct {
  blit_scaler_e scaler; // which scaler this table entry describes
  unsigned char valid;  // if >0, is valid option
  unsigned scale_x;     // whole-number X scale amount
  unsigned scale_y;     // whole-number Y scale amount
  char *desc_en;        // English short description
  // scaler_f           // function to invoke
  //                    // arg0 -> blit_scaler_e
  // scaler_f_arg1      // arg1 to function (to paramaterize the blitter functions)
  // scaler_f_arg2      // arg2 to function (to paramaterize the blitter functions)
} blit_scaler_option_t;

typedef enum {
  bs_invalid = 0,
  bs_valid
} blit_scaler_valid_e;

#endif
