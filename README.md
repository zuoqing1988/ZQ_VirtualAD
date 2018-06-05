ZQ_VirtualAD是一个植入虚拟广告的工具，包括区域跟踪，带透明度的抠图，还有一些其他的辅助功能。
依赖ZQlib，opencv2.4.13.5(特征点提取用的surf和sift，如果是opencv3需要修改包含的头文件)，ffmpeg-3.4.1（视频seek用的）， suitesparse-metis（抠图解方程用的）。
下载依赖库之后请自行在项目里面更改头文件路径和库路径。