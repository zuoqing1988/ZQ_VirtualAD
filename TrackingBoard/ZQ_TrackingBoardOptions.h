#ifndef _ZQ_TRACKING_BOARD_OPTIONS_H_
#define _ZQ_TRACKING_BOARD_OPTIONS_H_
#pragma once

namespace ZQ
{
	class ZQ_TrackingBoardOptions
	{
	public:
		enum CONST_VAL{FILENAME_LEN = 200};
		ZQ_TrackingBoardOptions()
		{
			Reset();
		}

		double reproj_err_thresh;
		int ransac_iter;
		int ransac_inner_iter;
		double ransac_confidence;
		int levmar_iter;
		double eps;
		double dis1_to_dis2_ratio;
		double feature_dis_angle;
		bool select_match_cross_check;
		bool display_running_info;
		bool export_debug_info;
		int max_keep_key_num;
		int ncores;
		

		//
		int width;
		int height;
		int refer_num;
		int frame_num;
		int base_id;
		char work_fold[FILENAME_LEN];
		std::vector<int> keyframe_id;

		void Reset()
		{
			reproj_err_thresh = 1.0;
			ransac_iter = 2000;
			ransac_inner_iter = 1;
			ransac_confidence = 1;
			levmar_iter = 300;
			eps = 1e-9;
			dis1_to_dis2_ratio = 0.6;
			feature_dis_angle = atan(1.0)*0.2;
			select_match_cross_check = false;
			display_running_info = false;
			export_debug_info = false;
			max_keep_key_num = 1000;
			ncores = 1;
			width = 480;
			height = 270;
			refer_num = 1;
			frame_num = 0;
			base_id = 0;
			strcpy(work_fold, ".");
		}

		bool HandleArgs(const int argc, const char** argv)
		{
			for (int k = 0; k < argc; k++)
			{
				if (_strcmpi(argv[k], "reproj_err_thresh") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k-1]);
						return false;
					}
					reproj_err_thresh = atof(argv[k]);
				}
				else if (_strcmpi(argv[k], "ransac_iter") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k-1]);
						return false;
					}
					ransac_iter = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "ransac_inner_iter") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					ransac_inner_iter = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "ransac_confidence") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k-1]);
						return false;
					}
					ransac_confidence = atof(argv[k]);
				}
				else if (_strcmpi(argv[k], "levmar_iter") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					levmar_iter = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "eps") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k-1]);
						return false;
					}
					eps = atof(argv[k]);
				}
				else if (_strcmpi(argv[k], "dis1_to_dis2_ratio") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k - 1]);
						return false;
					}
					dis1_to_dis2_ratio = atof(argv[k]);
				}
				else if (_strcmpi(argv[k], "feature_dis_angle") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k - 1]);
						return false;
					}
					feature_dis_angle = atof(argv[k]);
				}
				else if (_strcmpi(argv[k], "select_match_cross_check") == 0)
				{
					select_match_cross_check = true;
				}
				else if (_strcmpi(argv[k], "display") == 0)
				{
					display_running_info = true;
				}
				else if (_strcmpi(argv[k], "export_debug_info") == 0)
				{
					export_debug_info = true;
				}
				else if (_strcmpi(argv[k], "max_keep_key_num") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k - 1]);
						return false;
					}
					max_keep_key_num = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "ncores") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k - 1]);
						return false;
					}
					ncores = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "width") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k - 1]);
						return false;
					}
					width = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "height") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k - 1]);
						return false;
					}
					height = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "refer_num") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k - 1]);
						return false;
					}
					refer_num = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "frame_num") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k - 1]);
						return false;
					}
					frame_num = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "base_id") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k - 1]);
						return false;
					}
					base_id = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "work_fold") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k - 1]);
						return false;
					}
					strcpy(work_fold, argv[k]);
				}
				else if (_strcmpi(argv[k], "keyframe_id") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s\n", argv[k - 1]);
						return false;
					}
					keyframe_id.push_back(atoi(argv[k]));
				}
				else
				{
					printf("unknown parameters %s\n", argv[k]);
					return false;
				}
			}
			return true;
		}
	};
}

#endif