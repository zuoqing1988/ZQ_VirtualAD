#ifndef _ZQ_PLAY_TRACKING_CONFIG_H_
#define _ZQ_PLAY_TRACKING_CONFIG_H_
#pragma once 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

namespace ZQ
{
	class ZQ_PlayTrackingConfig
	{
		enum CONST_VAL{ FILE_NAME_LEN = 200 };
	public:
		char work_fold[FILE_NAME_LEN];
		char image_fold[FILE_NAME_LEN];
		char image_suffix[FILE_NAME_LEN];
		char key_fold[FILE_NAME_LEN];
		char key_suffix[FILE_NAME_LEN];
		char poly_fold[FILE_NAME_LEN];
		char marker_fold[FILE_NAME_LEN];
		char Hconfig_fold[FILE_NAME_LEN];

		int base_id;
		int frame_num;
		int max_propagate_polygon_num;
		int max_keep_key_num;
		int ncores;
		float match_dis1_to_dis2_ratio;
		float match_feature_dis_angle_degree;
		std::vector<int> poly_keyframes;
		std::vector<int> marker_keyframes;

	public:

		ZQ_PlayTrackingConfig()
		{
			Reset();
		}

		void Reset()
		{
			work_fold[0] = '\0';
			strcpy(image_fold, "images");
			strcpy(image_suffix, "jpg");
			strcpy(key_fold, "key");
			strcpy(key_suffix, "keyb");
			strcpy(poly_fold, "polyconfig");
			strcpy(marker_fold, "marker");
			strcpy(Hconfig_fold, "Hconfig");
			base_id = 0;
			frame_num = 0;
			max_propagate_polygon_num = 50;
			max_keep_key_num = 1000;
			ncores = 1;
			match_dis1_to_dis2_ratio = 0.6;
			match_feature_dis_angle_degree = 13.5;
			poly_keyframes.clear();
			marker_keyframes.clear();
		}

		bool LoadFromFile(const char* file)
		{
			FILE* in = fopen(file, "r");
			if (in == 0)
				return false;
			poly_keyframes.clear();
			marker_keyframes.clear();
			char cmdline[2000] = { 0 }, str1[2000], str2[2000];
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
				if (_strcmpi(str1, "work_fold") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(work_fold, pos + 1);
				}
				else if (_strcmpi(str1, "image_fold") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(image_fold, pos + 1);
				}
				else if (_strcmpi(str1, "image_suffix") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(image_suffix, pos + 1);
				}
				else if (_strcmpi(str1, "key_fold") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(key_fold, pos + 1);
				}
				else if (_strcmpi(str1, "key_suffix") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(key_suffix, pos + 1);
				}
				else if (_strcmpi(str1, "poly_fold") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(poly_fold, pos + 1);
				}
				else if (_strcmpi(str1, "marker_fold") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(marker_fold, pos + 1);
				}
				else if (_strcmpi(str1, "Hconfig_fold") == 0)
				{
					char* pos = strstr(cmdline, ":");
					if (pos == 0)
					{
						fclose(in);
						return false;
					}
					strcpy(Hconfig_fold, pos + 1);
				}
				else if (_strcmpi(str1, "base_id") == 0)
				{
					sscanf(cmdline, "%s%d", str1, &base_id);
				}
				else if (_strcmpi(str1, "frame_num") == 0)
				{
					sscanf(cmdline, "%s%d", str1, &frame_num);
				}
				else if (_strcmpi(str1, "max_propagate_polygon_num") == 0)
				{
					sscanf(cmdline, "%s%d", str1, &max_propagate_polygon_num);
				}
				else if (_strcmpi(str1, "max_keep_key_num") == 0)
				{
					sscanf(cmdline, "%s%d", str1, &max_keep_key_num);
				}
				else if (_strcmpi(str1, "ncores") == 0)
				{
					sscanf(cmdline, "%s%d", str1, &ncores);
				}
				else if (_strcmpi(str1, "match_dis1_to_dis2_ratio") == 0)
				{
					sscanf(cmdline, "%s%f", str1, &match_dis1_to_dis2_ratio);
				}
				else if (_strcmpi(str1, "match_feature_dis_angle_degree") == 0)
				{
					sscanf(cmdline, "%s%f", str1, &match_feature_dis_angle_degree);
				}
				else if (_strcmpi(str1, "poly_keyframe_id") == 0)
				{
					int tmp_id;
					sscanf(cmdline, "%s%d", str1, &tmp_id);
					poly_keyframes.push_back(tmp_id);
				}
				else if (_strcmpi(str1, "marker_keyframe_id") == 0)
				{
					int tmp_id;
					sscanf(cmdline, "%s%d", str1, &tmp_id);
					marker_keyframes.push_back(tmp_id);
				}
				
			}
			fclose(in);
			return true;
		}

		bool WriteToFiLe(const char* file)
		{
			FILE* out = fopen(file, "w");
			if (out == 0)
				return false;

			fprintf(out, "%s :%s\n", "work_fold", work_fold);
			fprintf(out, "%s :%s\n", "image_fold", image_fold);
			fprintf(out, "%s :%s\n", "image_suffix", image_suffix);
			fprintf(out, "%s :%s\n", "key_fold", key_fold);
			fprintf(out, "%s :%s\n", "poly_fold", poly_fold);
			fprintf(out, "%s :%s\n", "marker_fold", marker_fold);
			fprintf(out, "%s :%s\n", "Hconfig_fold", Hconfig_fold);
			fprintf(out, "%s %d\n", "base_id", base_id);
			fprintf(out, "%s %d\n", "frame_num", frame_num);
			fprintf(out, "%s %d\n", "max_propagate_polygon_num", max_propagate_polygon_num);
			fprintf(out, "%s %d\n", "max_keep_key_num", max_keep_key_num);
			fprintf(out, "%s %d\n", "ncores", ncores);
			fprintf(out, "%s %f\n", "match_dis1_to_dis2_ratio", match_dis1_to_dis2_ratio);
			fprintf(out, "%s %f\n", "match_feature_dis_angle_degree", match_feature_dis_angle_degree);
			for (int i = 0; i < poly_keyframes.size(); i++)
			{
				fprintf(out, "%s %d\n", "poly_keyframe_id", poly_keyframes[i]);
			}
			for (int i = 0; i < marker_keyframes.size(); i++)
			{
				fprintf(out, "%s %d\n", "marker_keyframe_id", marker_keyframes[i]);
			}
			fclose(out);
			return true;
		}
	};
}

#endif