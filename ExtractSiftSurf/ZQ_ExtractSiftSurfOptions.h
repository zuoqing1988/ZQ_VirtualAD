#ifndef _ZQ_EXTRACT_SIFT_SURF_OPTIONS_H_
#define _ZQ_EXTRACT_SIFT_SURF_OPTIONS_H_
#pragma once

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

namespace ZQ
{
	class ZQ_ExtractSiftSurfOptions
	{
		enum CONST_VAL{ FILENAME_MAX_LEN = 200 };
	public:
		enum MethodType
		{ 
			METHOD_SIFT, 
			METHOD_SURF 
		};

		MethodType methodType;
		float sift_contrast_thresh;
		float surf_hessian_thresh;
		char image_fold[FILENAME_MAX_LEN];
		char image_suffix[FILENAME_MAX_LEN];
		bool has_foremask;
		char foremask_fold[FILENAME_MAX_LEN];
		char foremask_suffix[FILENAME_MAX_LEN];
		char key_fold[FILENAME_MAX_LEN];
		char key_suffix[FILENAME_MAX_LEN];
		int base_id;
		int frame_num;
		int ncores;
		bool done_flag;

		ZQ_ExtractSiftSurfOptions()
		{
			Reset();
		}

		void Reset()
		{
			methodType = METHOD_SIFT;
			sift_contrast_thresh = 0.04;
			surf_hessian_thresh = 100;
			image_fold[0] = '\0';
			strcpy(image_suffix, "bmp");
			has_foremask = false;
			foremask_fold[0] = '\0';
			strcpy(foremask_suffix, "bmp");
			key_fold[0] = '\0';
			strcpy(key_suffix, "keyb");
			base_id = 0;
			frame_num = 0;
			ncores = 1;
			done_flag = false;
		}


		bool HandleArgs(const int argc, const char** argv)
		{
			for (int k = 0; k < argc; k++)
			{
				if (_strcmpi(argv[k], "methodType") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					if (_strcmpi(argv[k], "sift") == 0)
					{
						methodType = METHOD_SIFT;
					}
					else if (_strcmpi(argv[k], "surf") == 0)
					{
						methodType = METHOD_SURF;
					}
					else
					{
						printf("unknown MethodType: %s\n", argv[k]);
						return false;
					}
				}
				else if (_strcmpi(argv[k], "sift_contrast_thresh") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					sift_contrast_thresh = atof(argv[k]);
				}
				else if (_strcmpi(argv[k], "surf_hessian_thresh") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					surf_hessian_thresh = atof(argv[k]);
				}
				else if (_strcmpi(argv[k], "image_fold") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(image_fold, argv[k]);
				}
				else if (_strcmpi(argv[k], "image_suffix") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(image_suffix, argv[k]);
				}
				else if (_strcmpi(argv[k], "foremask_fold") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(foremask_fold, argv[k]);
					has_foremask = true;
				}
				else if (_strcmpi(argv[k], "foremask_suffix") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(foremask_suffix, argv[k]);
				}
				else if (_strcmpi(argv[k], "key_fold") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(key_fold, argv[k]);
				}
				else if (_strcmpi(argv[k], "key_suffix") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(key_suffix, argv[k]);
				}
				else if (_strcmpi(argv[k], "base_id") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					base_id = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "frame_num") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					frame_num = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "ncores") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					ncores = atoi(argv[k]);
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