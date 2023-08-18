// Parameters for KITTI dataset

// The number of superpixel(决定了图像以多少超像素分割)
int superpixelTotal = 5000;

// The number of iterations（决定超像素分割后的的平滑程度）
int outerIterationTotal = 100;   //处理平滑分割的总次数
int innerIterationTotal = 10;      //处理平面平滑的次数（总次数等于二者乘积）

// Weight parameters（一些有关一致性的权重参数）
double lambda_pos = 1000.0; //位置
double lambda_depth = 3000.0;  //深度
double lambda_bou = 1000.0;     //边界
double lambda_smo = 400.0;      //平滑度

// Inlier threshold
//double lambda_d = 3.0;      //用到它的地方，均可理解为当时计算的误差比它大 ，异常标记值就为255，为最亮，即outlierFlagImage_[width_*y + x] = 255;
double lambda_d = 3.0;

// Penalty values（可以理解为越大越不会过拟合）
double lambda_hinge = 5.0;      //控制铰链惩罚参数
double lambda_occ = 150.0;       //控制遮挡惩罚参数
double lambda_pen = 30.0;       //控制不可能情况惩罚参数
