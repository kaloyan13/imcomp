#include "vl_register_images.h"

int main(int argc, char** argv) {
  Magick::InitializeMagick(*argv);

  string compare_id = "test_vl_register_images";
  boost::filesystem::path upload_dir_("/home/tlm/exp/imcomp/images/traherne_book/set2/");
  boost::filesystem::path result_dir_("/home/tlm/exp/imcomp/images/traherne_book/set2/result/");

  string fid1 = "im1";
  string fid2 = "im2";

  boost::filesystem::path im1_fn = upload_dir_ / (fid1 + ".jpg");
  boost::filesystem::path im2_fn = upload_dir_ / (fid2 + ".jpg");
  boost::filesystem::path im1_out_fn = result_dir_ / (fid1 + "_" + compare_id + "_crop.jpg");
  boost::filesystem::path im2_out_fn = result_dir_ / (fid2 + "_" + compare_id + + "_crop.jpg");
  boost::filesystem::path im2_tx_fn  = result_dir_ / (fid2 + "_" + compare_id + + "_crop_tx.jpg");
  boost::filesystem::path diff_fn    = result_dir_ / (fid1 + "_" + fid2 + "_" + compare_id + "_diff.jpg");
  boost::filesystem::path overlap_fn    = result_dir_ / (fid1 + "_" + fid2 + "_" + compare_id + "_overlap.jpg");

  uint32_t best_inliers_count = -1;
  MatrixXd H(3,3);

  unsigned int file1_region[4] = {0, 0, 550, 482}; // x0, y0, x1, y1

  vl_register_images::register_images( im1_fn.string().c_str(),
                                     im2_fn.string().c_str(),
                                     file1_region[0], file1_region[2], file1_region[1], file1_region[3],
                                     H, best_inliers_count,
                                     im1_out_fn.string().c_str(),
                                     im2_out_fn.string().c_str(),
                                     im2_tx_fn.string().c_str(),
                                     diff_fn.string().c_str(),
                                     overlap_fn.string().c_str()
                                     );

  return 0;
}
