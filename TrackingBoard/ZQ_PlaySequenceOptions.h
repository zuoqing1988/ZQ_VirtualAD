#ifndef _ZQ_PLAY_SEQUENCE_OPTIONS_H_
#define _ZQ_PLAY_SEQUENCE_OPTIONS_H_
#pragma once
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
namespace ZQ
{
	class ZQ_PlaySequenceOptions
	{
		enum CONST_VAL{ FILE_NAME_LEN = 200 };
	public:
		char work_fold[FILE_NAME_LEN];
		char image_fold[FILE_NAME_LEN];
		char image_suffix[FILE_NAME_LEN];
		char poly_fold[FILE_NAME_LEN];
		char marker_fold[FILE_NAME_LEN];
		int base_id;
		int frame_num;
	
	public:
		ZQ_PlaySequenceOptions()
		{
			Reset();
		}

		void Reset()
		{
			work_fold[0] = '\0';
			strcpy(image_fold, "images");
			strcpy(image_suffix, "jpg");
			strcpy(poly_fold, "polyconfig");
			strcpy(marker_fold, "marker");
			base_id = 0;
			frame_num = 0;
		}

		bool HandleArgs(const int argc, const char** argv)
		{
			for (int k = 0; k < argc; k++)
			{
				if (_strcmpi(argv[k], "work_fold") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(work_fold, argv[k]);
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
				else if (_strcmpi(argv[k], "poly_fold") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(poly_fold, argv[k]);
				}
				else if (_strcmpi(argv[k], "marker_fold") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(marker_fold, argv[k]);
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