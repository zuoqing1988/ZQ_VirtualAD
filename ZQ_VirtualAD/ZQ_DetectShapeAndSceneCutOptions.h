#ifndef _ZQ_DETECT_SHAPE_AND_SCENE_CUT_OPTIONS_H_
#define _ZQ_DETECT_SHAPE_AND_SCENE_CUT_OPTIONS_H_
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace ZQ
{
	class ZQ_DetectShapeAndSceneCutOptions
	{
		enum CONST_VAL{MAX_FILENAME_LEN = 200};
	public:
		char video_file[MAX_FILENAME_LEN];
		bool has_foremask_file;
		char foremask_file[MAX_FILENAME_LEN];
		bool export_thumbnails;
		char thumbnails_fold[MAX_FILENAME_LEN];
		float thumbnails_scale;
		bool export_shape_polygon;
		char shape_polygon_fold[MAX_FILENAME_LEN];
		bool remove_repeat_shape;
		bool export_scene_cut_config;
		char scene_cut_config_file[MAX_FILENAME_LEN];
		bool export_scene_cut_videos;
		char scene_cut_video_fold[MAX_FILENAME_LEN];
		float canny_thresh;
		int canny_aperture_size;
		int binary_Nlevels;
		float shape_min_area;
		float shape_angle_ratio;
		int shape_feature_num_thresh;
		float repeat_shape_dis_thresh;	//for remove repeat shape in one frame
		bool display_running_info;
		int detect_shape_skip;
		float dis_avg_L2_thresh;
		float dis_hist_thresh;
		int ncores;
		float static_pixels_ratio;
		//for remove repeat shape between frames


		ZQ_DetectShapeAndSceneCutOptions()
		{
			Reset();
		}

		void Reset()
		{
			video_file[0] = '\0';
			has_foremask_file = false;
			foremask_file[0] = '\0';
			export_thumbnails = false;
			thumbnails_fold[0] = '\0';
			thumbnails_scale = 0.5;
			export_shape_polygon = false;
			shape_polygon_fold[0] = '\0';
			remove_repeat_shape = false;
			export_scene_cut_config = false;
			scene_cut_config_file[0] = '\0';
			export_scene_cut_videos = false;
			scene_cut_video_fold[0] = '\0';
			canny_thresh = 400;
			canny_aperture_size = 5;
			binary_Nlevels = 16;
			shape_min_area = 2500;
			shape_angle_ratio = 0.25;
			shape_feature_num_thresh = 25;
			repeat_shape_dis_thresh = 10;
			display_running_info = false;
			detect_shape_skip = 25;
			dis_avg_L2_thresh = 10;
			dis_hist_thresh = 0.7;
			ncores = 1;
			static_pixels_ratio = 0.8;
		}

		bool LoadFromFile(const char* file)
		{
			FILE* in = fopen(file, "r");
			if (in == 0)
				return false;
			char cmdline[2000] = { 0 }, str1[2000];
			while (true)
			{
				strcpy(cmdline, "");
				fgets(cmdline, 1999, in);
				if (strcmp(cmdline, "") == 0)
					break;
				int len = strlen(cmdline);
				if (cmdline[len - 1] == '\n')
				{
					cmdline[--len] = '\0';
				}

				if (cmdline[0] == '\0' || cmdline[0] == '#')
					continue;

				sscanf(cmdline, "%s", str1);
				if (_strcmpi(str1, "video_file") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(video_file, pos + 1);
				}
				else if (_strcmpi(str1,"has_foremask_file") == 0)
				{
					int v;
					sscanf(cmdline, "%s%d", str1, &v);
					has_foremask_file = v != 0;
				}
				else if (_strcmpi(str1, "foremask_file") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(foremask_file, pos + 1);
				}
				else if (_strcmpi(str1, "export_thumbnails") == 0)
				{
					int v;
					sscanf(cmdline, "%s%d", str1, &v);
					export_thumbnails = v != 0;
				}
				else if (_strcmpi(str1, "thumbnails_fold") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(thumbnails_fold, pos + 1);
				}
				else if (_strcmpi(str1, "thumbnails_scale") == 0)
				{
					sscanf(cmdline, "%s%f", str1, &thumbnails_scale);
				}
				else if (_strcmpi(str1, "export_shape_polygon") == 0)
				{
					int v;
					sscanf(cmdline, "%s%d", str1, &v);
					export_shape_polygon = v != 0;
				}
				else if (_strcmpi(str1, "shape_polygon_fold") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(shape_polygon_fold, pos + 1);
				}
				else if (_strcmpi(str1, "remove_repeat_shape") == 0)
				{
					int v;
					sscanf(cmdline, "%s%d", str1, &v);
					remove_repeat_shape = v != 0;
				}
				else if (_strcmpi(str1, "export_scene_cut_config") == 0)
				{
					int v;
					sscanf(cmdline, "%s%d", str1, &v);
					export_scene_cut_config = v != 0;
				}
				else if (_strcmpi(str1, "scene_cut_config_file") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(scene_cut_config_file, pos + 1);
				}
				else if (_strcmpi(str1, "export_scene_cut_videos") == 0)
				{
					int v;
					sscanf(cmdline, "%s%d", str1, &v);
					export_scene_cut_videos = v != 0;
				}
				else if (_strcmpi(str1, "scene_cut_video_fold") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(scene_cut_video_fold, pos + 1);
				}
				else if (_strcmpi(str1, "canny_thresh") == 0)
				{
					sscanf(cmdline, "%s%f", str1, &canny_thresh);
				}
				else if (_strcmpi(str1, "canny_aperture_size") == 0)
				{
					sscanf(cmdline, "%s%d", str1, &canny_aperture_size);
				}
				else if (_strcmpi(str1, "binary_Nlevels") == 0)
				{
					sscanf(cmdline, "%s%d", str1, &binary_Nlevels);
				}
				else if (_strcmpi(str1, "shape_min_area") == 0)
				{
					sscanf(cmdline, "%s%f", str1, &shape_min_area);
				}
				else if (_strcmpi(str1, "shape_angle_ratio") == 0)
				{
					sscanf(cmdline, "%s%f", str1, &shape_angle_ratio);
				}
				else if (_strcmpi(str1, "shape_feature_num_thresh") == 0)
				{
					sscanf(cmdline, "%s%d", str1, &shape_feature_num_thresh);
				}
				else if (_strcmpi(str1, "repeat_shape_dis_thresh") == 0)
				{
					sscanf(cmdline, "%s%f", str1, &repeat_shape_dis_thresh);
				}
				else if (_strcmpi(str1, "display_running_info") == 0)
				{
					int v;
					sscanf(cmdline, "%s%d", str1, &v);
					display_running_info = v != 0;
				}
				else if (_strcmpi(str1, "detect_shape_skip") == 0)
				{
					sscanf(cmdline, "%s%d", str1, &detect_shape_skip);
				}
				else if (_strcmpi(str1, "dis_avg_L2_thresh") == 0)
				{
					sscanf(cmdline, "%s%f", str1, &dis_avg_L2_thresh);
				}
				else if (_strcmpi(str1, "dis_hist_thresh") == 0)
				{
					sscanf(cmdline, "%s%f", str1, &dis_hist_thresh);
				}
				else if (_strcmpi(str1, "ncores") == 0)
				{
					sscanf(cmdline, "%s%d", str1, &ncores);
				}
				else if (_strcmpi(str1, "static_pixels_ratio") == 0)
				{
					sscanf(cmdline, "%s%f", str1, &static_pixels_ratio);
				}
				
			}
			fclose(in);
			return true;
		}
	};
}

#endif