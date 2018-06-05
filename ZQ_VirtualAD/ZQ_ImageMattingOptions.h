#ifndef _ZQ_IMAGE_MATTING_OPTIONS_H_
#define _ZQ_IMAGE_MATTING_OPTIONS_H_
#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
namespace ZQ
{
	class ZQ_ImageMattingOptions
	{
		enum CONST_VAL{FILE_NAME_LEN = 200};
	public:
		enum RenderMode{RENDER_NOTHING, RENDER_ONLY_TEX, RENDER_TEX_WITH_ALPHA, RENDER_FBA_TEX, RENDER_FBA_TEX_WITH_ALPHA};
		bool has_in_image;
		char in_image_file[FILE_NAME_LEN];
		bool has_in_trimap;
		char in_trimap_file[FILE_NAME_LEN];
		bool has_in_marker;
		char in_marker_file[FILE_NAME_LEN];
		bool has_in_tex;
		char in_tex_file[FILE_NAME_LEN];
		bool has_in_tex_alpha;
		char in_tex_alpha_file[FILE_NAME_LEN];
		bool has_out_fore;
		char out_fore_file[FILE_NAME_LEN];
		bool has_out_back;
		char out_back_file[FILE_NAME_LEN];
		bool has_out_alpha;
		char out_alpha_file[FILE_NAME_LEN];
		bool has_out_render;
		char out_render_file[FILE_NAME_LEN];
		char marker_mode[FILE_NAME_LEN];
		RenderMode render_mode;
		bool auto_matting;
		int auto_matting_back_erode_size;
		int auto_matting_fore_erode_size;
		int auto_matting_board_dilate_size;
		int win_size;
	
		ZQ_ImageMattingOptions()
		{
			Reset();
		}
		void Reset()
		{
			has_in_image = false; 
			in_image_file[0] = '\0';
			has_in_trimap = false;
			in_trimap_file[0] = '\0';
			has_in_marker = false;
			in_marker_file[0] = '\0';
			has_in_tex = false;
			in_tex_file[0] = '\0';
			has_in_tex_alpha = false;
			in_tex_alpha_file[0] = '\0';
			has_out_fore = false;
			strcpy(out_fore_file, "fore_1234567890.png");
			has_out_back = false;
			strcpy(out_back_file, "back_1234567890.png");
			has_out_alpha = false;
			strcpy(out_alpha_file, "alpha_1234567890.png");
			has_out_render = false;
			strcpy(out_render_file, "render_1234567890.png");
			strcpy(marker_mode, "abcd");
			render_mode = RENDER_NOTHING;
			auto_matting = false;
			auto_matting_back_erode_size = 10;
			auto_matting_fore_erode_size = 10;
			auto_matting_board_dilate_size = 10;
			win_size = 1;
		}

		bool HandleArgs(const int argc, const char** argv)
		{
			for (int k = 0; k < argc; k++)
			{
				if (_strcmpi(argv[k], "in_image_file") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(in_image_file, argv[k]);
					has_in_image = true;
				}
				else if (_strcmpi(argv[k], "in_trimap_file") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(in_trimap_file, argv[k]);
					has_in_trimap = true;
				}
				else if (_strcmpi(argv[k], "in_marker_file") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(in_marker_file, argv[k]);
					has_in_marker = true;
				}
				else if (_strcmpi(argv[k], "in_tex_file") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(in_tex_file, argv[k]);
					has_in_tex = true;
				}
				else if (_strcmpi(argv[k], "in_tex_alpha_file") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(in_tex_alpha_file, argv[k]);
					has_in_tex_alpha = true;
				}
				else if (_strcmpi(argv[k], "out_fore_file") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(out_fore_file, argv[k]);
					has_out_fore = true;
				}
				else if (_strcmpi(argv[k], "out_back_file") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(out_back_file, argv[k]);
					has_out_back = true;
				}
				else if (_strcmpi(argv[k], "out_alpha_file") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(out_alpha_file, argv[k]);
					has_out_alpha = true;
				}
				else if (_strcmpi(argv[k], "out_render_file") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(out_render_file, argv[k]);
					has_out_render = true;
				}
				else if (_strcmpi(argv[k], "marker_mode") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					strcpy(marker_mode, argv[k]);
				}
				else if (_strcmpi(argv[k], "render_mode") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					if (_strcmpi(argv[k], "nothing") == 0)
					{
						render_mode = RENDER_NOTHING;
					}
					else if (_strcmpi(argv[k], "tex") == 0)
					{
						render_mode = RENDER_ONLY_TEX;
					}
					else if (_strcmpi(argv[k], "tex_with_alpha") == 0)
					{
						render_mode = RENDER_TEX_WITH_ALPHA;
					}
					else if (_strcmpi(argv[k], "fba_tex") == 0)
					{
						render_mode = RENDER_FBA_TEX;
					}
					else if (_strcmpi(argv[k], "fba_tex_with_alpha") == 0)
					{
						render_mode = RENDER_FBA_TEX_WITH_ALPHA;
					}
					else
					{
						printf("unknown RenderMode %s\n", argv[k]);
						return false;
					}
				}
				else if (_strcmpi(argv[k], "auto_matting") == 0)
				{
					auto_matting = true;
				}
				else if (_strcmpi(argv[k], "back_erode_size") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					auto_matting_back_erode_size = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "fore_erode_size") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					auto_matting_fore_erode_size = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "board_dilate_size") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					auto_matting_board_dilate_size = atoi(argv[k]);
				}
				else if (_strcmpi(argv[k], "win_size") == 0)
				{
					k++;
					if (k >= argc)
					{
						printf("the value of %s ?\n", argv[k - 1]);
						return false;
					}
					win_size = atoi(argv[k]);
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