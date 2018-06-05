#ifndef _ZQ_VIRTUAL_ADVERTISEMENT_RENDER_H_
#define _ZQ_VIRTUAL_ADVERTISEMENT_RENDER_H_
#pragma once
#include "ZQ_DoubleImage.h"
#include "ZQ_CPURenderer2DWorkspace.h"
namespace ZQ
{
	class ZQ_VirtualAdvertisementRender
	{
	public:
		enum MarkerMode{ MODE_ABCD, MODE_BCDA, MODE_CDAB, MODE_DABC, MODE_ADCB, MODE_DCBA, MODE_CBAD, MODE_BADC };
		static bool GetMarkerMode(const char* str, MarkerMode& mode)
		{
			if (str == 0)
				return false;
			if (_strcmpi(str, "abcd") == 0)
				mode = MODE_ABCD;
			else if (_strcmpi(str, "bcda") == 0)
				mode = MODE_BCDA;
			else if (_strcmpi(str, "cdab") == 0)
				mode = MODE_CDAB;
			else if (_strcmpi(str, "dabc") == 0)
				mode = MODE_DABC;
			else if (_strcmpi(str, "adcb") == 0)
				mode = MODE_ADCB;
			else if (_strcmpi(str, "dcba") == 0)
				mode = MODE_DCBA;
			else if (_strcmpi(str, "cbad") == 0)
				mode = MODE_CBAD;
			else if (_strcmpi(str, "badc") == 0)
				mode = MODE_BADC;
			else
			{
				return false;
			}
			return true;
		}

		static bool Render(ZQ_DImage<float>& out_im, const ZQ_DImage<float>& im, const ZQ_DImage<float>& tex, const double marker[8], MarkerMode mode)
		{
			int width = im.width();
			int height = im.height();
			int nChannels = im.nchannels();
			if (width <= 0 || height <= 0 || nChannels <= 0)
				return false;

			float vertices[20] =
			{
				0, 0, 10, 0, 0,
				0, 1, 10, 0, 1,
				1, 1, 10, 1, 1,
				1, 0, 10, 1, 0,
			};
			int indices[6] =
			{
				0, 1, 2,
				0, 2, 3,
			};

			switch (mode)
			{
			case MODE_ABCD:
				vertices[0] = marker[0];	vertices[1] = marker[1];
				vertices[5] = marker[2];	vertices[6] = marker[3];
				vertices[10] = marker[4];	vertices[11] = marker[5];
				vertices[15] = marker[6];	vertices[16] = marker[7];
				break;
			case MODE_BCDA:
				vertices[5] = marker[0];	vertices[6] = marker[1];
				vertices[10] = marker[2];	vertices[11] = marker[3];
				vertices[15] = marker[4];	vertices[16] = marker[5];
				vertices[0] = marker[6];	vertices[1] = marker[7];
				break;
			case MODE_CDAB:
				vertices[10] = marker[0];	vertices[11] = marker[1];
				vertices[15] = marker[2];	vertices[16] = marker[3];
				vertices[0] = marker[4];	vertices[1] = marker[5];
				vertices[5] = marker[6];	vertices[6] = marker[7];
				break;
			case MODE_DABC:
				vertices[15] = marker[0];	vertices[16] = marker[1];
				vertices[0] = marker[2];	vertices[1] = marker[3];
				vertices[5] = marker[4];	vertices[6] = marker[5];
				vertices[10] = marker[6];	vertices[11] = marker[7];
				break;
			case MODE_ADCB:
				vertices[0] = marker[0];	vertices[1] = marker[1];
				vertices[15] = marker[2];	vertices[16] = marker[3];
				vertices[10] = marker[4];	vertices[11] = marker[5];
				vertices[5] = marker[6];	vertices[6] = marker[7];
				break;
			case MODE_DCBA:
				vertices[15] = marker[0];	vertices[16] = marker[1];
				vertices[10] = marker[2];	vertices[11] = marker[3];
				vertices[5] = marker[4];	vertices[6] = marker[5];
				vertices[0] = marker[6];	vertices[1] = marker[7];
				break;
			case MODE_CBAD:
				vertices[10] = marker[0];	vertices[11] = marker[1];
				vertices[5] = marker[2];	vertices[6] = marker[3];
				vertices[0] = marker[4];	vertices[1] = marker[5];
				vertices[15] = marker[6];	vertices[16] = marker[7];
				break;
			case MODE_BADC:
				vertices[5] = marker[0];	vertices[6] = marker[1];
				vertices[0] = marker[2];	vertices[1] = marker[3];
				vertices[15] = marker[4];	vertices[16] = marker[5];
				vertices[10] = marker[6];	vertices[11] = marker[7];
				break;
			}

			ZQ_TextureSampler<float> sampler;
			sampler.BindImage(tex, false);
			ZQ_CPURenderer2DWorkspace render2D(width, height);
			render2D.ClearDepthBuffer(1000);
			render2D.SetClip(1, 1000);
			render2D.BindSampler(&sampler);
			render2D.DisableAlphaBlend();
			render2D.SetBackground(im);
			render2D.RenderIndexedTriangles(vertices, indices, 4, 2, ZQ_CPURenderer3DWorkspace::VERTEX_POSITION3_TEXCOORD2);
			const float*& buffer_data2D = render2D.GetColorBufferPtr();
			out_im.allocate(width, height, nChannels);
			float*& out_data = out_im.data();
			for (int h = 0; h < height; h++)
			{
				for (int w = 0; w < width; w++)
				{
					memcpy(out_data + (h*width + w)*nChannels, buffer_data2D + (h*width + w) * 4, sizeof(float)*nChannels);
				}
			}
			return true;
		}

		static bool Render_with_alpha(ZQ_DImage<float>& out_im, const ZQ_DImage<float>& im, const ZQ_DImage<float>& tex, const ZQ_DImage<float>& tex_alpha, const double marker[4], MarkerMode mode)
		{			
			int tex_width = tex.width();
			int tex_height = tex.height();
			int tex_nChannels = tex.nchannels();
			if (tex_alpha.width() != tex_width || tex_alpha.height() != tex_height || tex_alpha.nchannels() != 1)
			{
				printf("dimension of tex image and alpha dont match!\n");
				return false;
			}

			int width = im.width();
			int height = im.height();
			int nChannels = im.nchannels();
			if (width <= 0 || height <= 0 || nChannels <= 0)
				return false;

			float vertices[20] =
			{
				0, 0, 10, 0, 0,
				0, 1, 10, 0, 1,
				1, 1, 10, 1, 1,
				1, 0, 10, 1, 0,
			};
			int indices[6] =
			{
				0, 1, 2,
				0, 2, 3,
			};

			switch (mode)
			{
			case MODE_ABCD:
				vertices[0] = marker[0];	vertices[1] = marker[1];
				vertices[5] = marker[2];	vertices[6] = marker[3];
				vertices[10] = marker[4];	vertices[11] = marker[5];
				vertices[15] = marker[6];	vertices[16] = marker[7];
				break;
			case MODE_BCDA:
				vertices[5] = marker[0];	vertices[6] = marker[1];
				vertices[10] = marker[2];	vertices[11] = marker[3];
				vertices[15] = marker[4];	vertices[16] = marker[5];
				vertices[0] = marker[6];	vertices[1] = marker[7];
				break;
			case MODE_CDAB:
				vertices[10] = marker[0];	vertices[11] = marker[1];
				vertices[15] = marker[2];	vertices[16] = marker[3];
				vertices[0] = marker[4];	vertices[1] = marker[5];
				vertices[5] = marker[6];	vertices[6] = marker[7];
				break;
			case MODE_DABC:
				vertices[15] = marker[0];	vertices[16] = marker[1];
				vertices[0] = marker[2];	vertices[1] = marker[3];
				vertices[5] = marker[4];	vertices[6] = marker[5];
				vertices[10] = marker[6];	vertices[11] = marker[7];
				break;
			case MODE_ADCB:
				vertices[0] = marker[0];	vertices[1] = marker[1];
				vertices[15] = marker[2];	vertices[16] = marker[3];
				vertices[10] = marker[4];	vertices[11] = marker[5];
				vertices[5] = marker[6];	vertices[6] = marker[7];
				break;
			case MODE_DCBA:
				vertices[15] = marker[0];	vertices[16] = marker[1];
				vertices[10] = marker[2];	vertices[11] = marker[3];
				vertices[5] = marker[4];	vertices[6] = marker[5];
				vertices[0] = marker[6];	vertices[1] = marker[7];
				break;
			case MODE_CBAD:
				vertices[10] = marker[0];	vertices[11] = marker[1];
				vertices[5] = marker[2];	vertices[6] = marker[3];
				vertices[0] = marker[4];	vertices[1] = marker[5];
				vertices[15] = marker[6];	vertices[16] = marker[7];
				break;
			case MODE_BADC:
				vertices[5] = marker[0];	vertices[6] = marker[1];
				vertices[0] = marker[2];	vertices[1] = marker[3];
				vertices[15] = marker[4];	vertices[16] = marker[5];
				vertices[10] = marker[6];	vertices[11] = marker[7];
				break;
			}


			ZQ_DImage<float> tex_bgra(tex_width, tex_height, 4);
			float*& tex_bgra_data = tex_bgra.data();
			const float*& tex_data = tex.data();
			const float*& tex_alpha_data = tex_alpha.data();
			if (tex_nChannels == 1)
			{
				for (int h = 0; h < tex_height; h++)
				{
					for (int w = 0; w < tex_width; w++)
					{
						tex_bgra_data[(h*tex_width + w) * 4 + 0] = tex_data[h*tex_width + w];
						tex_bgra_data[(h*tex_width + w) * 4 + 1] = tex_data[h*tex_width + w];
						tex_bgra_data[(h*tex_width + w) * 4 + 2] = tex_data[h*tex_width + w];
						tex_bgra_data[(h*tex_width + w) * 4 + 3] = tex_alpha_data[h*tex_width + w];
					}
				}
			}
			else if (tex_nChannels == 3)
			{
				for (int h = 0; h < tex_height; h++)
				{
					for (int w = 0; w < tex_width; w++)
					{
						memcpy(tex_bgra_data + (h*tex_width + w) * 4, tex_data + (h*tex_width + w) * 3, sizeof(float)* 3);
						tex_bgra_data[(h*tex_width + w) * 4 + 3] = tex_alpha_data[h*tex_width + w];
					}
				}
			}
			else
			{
				printf("invalid tex nChannels!\n");
				return false;
			}
			ZQ_TextureSampler<float> sampler;
			sampler.BindImage(tex_bgra, false);
			ZQ_CPURenderer2DWorkspace render2D(width, height);
			render2D.ClearDepthBuffer(1000);
			render2D.DisableDepthTest();
			render2D.EnableAlphaBlend();
			render2D.SetAlphaBlendMode(ZQ_CPURenderer3DWorkspace::ALPHABLEND_SRC_ALPHA_DST_ONE_MINUS_SRC);
			render2D.SetClip(1, 1000);
			render2D.BindSampler(&sampler);
			render2D.SetBackground(im);
			render2D.RenderIndexedTriangles(vertices, indices, 4, 2, ZQ_CPURenderer3DWorkspace::VERTEX_POSITION3_TEXCOORD2);
			const float*& buffer_data2D = render2D.GetColorBufferPtr();
			out_im.allocate(width, height, nChannels);
			float*& out_data = out_im.data();
			for (int h = 0; h < height; h++)
			{
				for (int w = 0; w < width; w++)
				{
					memcpy(out_data + (h*width + w)*nChannels, buffer_data2D + (h*width + w) * 4, sizeof(float)*nChannels);
				}
			}
			return true;
		}

		static bool Render_fba(ZQ_DImage<float>& out_im, const ZQ_DImage<float>& fore, const ZQ_DImage<float>& back, const ZQ_DImage<float>& alpha, const ZQ_DImage<float>& tex, 
			const double marker[8], MarkerMode mode)
		{
			int width = fore.width();
			int height = fore.height();
			int nChannels = fore.nchannels();
			if (width <= 0 || height <= 0 || nChannels <= 0)
				return false;
			if (!back.matchDimension(width, height, nChannels) || !alpha.matchDimension(width, height, 1))
			{
				printf("dimension dont match!\n");
				return false;
			}

			float vertices[20] =
			{
				0, 0, 10, 0, 0,
				0, 1, 10, 0, 1,
				1, 1, 10, 1, 1,
				1, 0, 10, 1, 0,
			};
			int indices[6] =
			{
				0, 1, 2,
				0, 2, 3,
			};

			switch (mode)
			{
			case MODE_ABCD:
				vertices[0] = marker[0];	vertices[1] = marker[1];
				vertices[5] = marker[2];	vertices[6] = marker[3];
				vertices[10] = marker[4];	vertices[11] = marker[5];
				vertices[15] = marker[6];	vertices[16] = marker[7];
				break;
			case MODE_BCDA:
				vertices[5] = marker[0];	vertices[6] = marker[1];
				vertices[10] = marker[2];	vertices[11] = marker[3];
				vertices[15] = marker[4];	vertices[16] = marker[5];
				vertices[0] = marker[6];	vertices[1] = marker[7];
				break;
			case MODE_CDAB:
				vertices[10] = marker[0];	vertices[11] = marker[1];
				vertices[15] = marker[2];	vertices[16] = marker[3];
				vertices[0] = marker[4];	vertices[1] = marker[5];
				vertices[5] = marker[6];	vertices[6] = marker[7];
				break;
			case MODE_DABC:
				vertices[15] = marker[0];	vertices[16] = marker[1];
				vertices[0] = marker[2];	vertices[1] = marker[3];
				vertices[5] = marker[4];	vertices[6] = marker[5];
				vertices[10] = marker[6];	vertices[11] = marker[7];
				break;
			case MODE_ADCB:
				vertices[0] = marker[0];	vertices[1] = marker[1];
				vertices[15] = marker[2];	vertices[16] = marker[3];
				vertices[10] = marker[4];	vertices[11] = marker[5];
				vertices[5] = marker[6];	vertices[6] = marker[7];
				break;
			case MODE_DCBA:
				vertices[15] = marker[0];	vertices[16] = marker[1];
				vertices[10] = marker[2];	vertices[11] = marker[3];
				vertices[5] = marker[4];	vertices[6] = marker[5];
				vertices[0] = marker[6];	vertices[1] = marker[7];
				break;
			case MODE_CBAD:
				vertices[10] = marker[0];	vertices[11] = marker[1];
				vertices[5] = marker[2];	vertices[6] = marker[3];
				vertices[0] = marker[4];	vertices[1] = marker[5];
				vertices[15] = marker[6];	vertices[16] = marker[7];
				break;
			case MODE_BADC:
				vertices[5] = marker[0];	vertices[6] = marker[1];
				vertices[0] = marker[2];	vertices[1] = marker[3];
				vertices[15] = marker[4];	vertices[16] = marker[5];
				vertices[10] = marker[6];	vertices[11] = marker[7];
				break;
			}

			ZQ_TextureSampler<float> sampler;
			sampler.BindImage(tex, false);
			ZQ_CPURenderer2DWorkspace render2D(width, height);
			render2D.ClearDepthBuffer(1000);
			render2D.SetClip(1, 1000);
			render2D.BindSampler(&sampler);
			render2D.DisableAlphaBlend();
			render2D.SetBackground(back);
			render2D.RenderIndexedTriangles(vertices, indices, 4, 2, ZQ_CPURenderer3DWorkspace::VERTEX_POSITION3_TEXCOORD2);
			const float*& buffer_data2D = render2D.GetColorBufferPtr();
			out_im.allocate(width, height, nChannels);
			float*& out_data = out_im.data();
			for (int h = 0; h < height; h++)
			{
				for (int w = 0; w < width; w++)
				{
					memcpy(out_data + (h*width + w)*nChannels, buffer_data2D + (h*width + w) * 4, sizeof(float)*nChannels);
				}
			}


			const float*& alpha_data = alpha.data();
			const float*& fore_data = fore.data();
			for (int i = 0; i < width*height; i++)
			{
				for (int c = 0; c < nChannels; c++)
				{
					out_data[i*nChannels + c] *= 1 - alpha_data[i];
					out_data[i*nChannels + c] += fore_data[i*nChannels + c] * alpha_data[i];
				}
			}
			return true;
		}

		static bool Render_fba_with_alpha(ZQ_DImage<float>& out_im, const ZQ_DImage<float>& fore, const ZQ_DImage<float>& back, const ZQ_DImage<float>& alpha, const ZQ_DImage<float>& tex,
			const ZQ_DImage<float>& tex_alpha, const double marker[8], MarkerMode mode)
		{
			int width = fore.width();
			int height = fore.height();
			int nChannels = fore.nchannels();
			if (width <= 0 || height <= 0 || nChannels <= 0)
				return false;
			if (!back.matchDimension(width, height, nChannels) || !alpha.matchDimension(width, height, 1))
			{
				printf("dimension dont match!\n");
				return false;
			}

			int tex_width = tex.width();
			int tex_height = tex.height();
			int tex_nChannels = tex.nchannels();
			if (tex_alpha.width() != tex_width || tex_alpha.height() != tex_height)
			{
				printf("dimension of tex image and alpha dont match!\n");
				return -1;
			}

			float vertices[20] =
			{
				0, 0, 10, 0, 0,
				0, 1, 10, 0, 1,
				1, 1, 10, 1, 1,
				1, 0, 10, 1, 0,
			};
			int indices[6] =
			{
				0, 1, 2,
				0, 2, 3,
			};

			switch (mode)
			{
			case MODE_ABCD:
				vertices[0] = marker[0];	vertices[1] = marker[1];
				vertices[5] = marker[2];	vertices[6] = marker[3];
				vertices[10] = marker[4];	vertices[11] = marker[5];
				vertices[15] = marker[6];	vertices[16] = marker[7];
				break;
			case MODE_BCDA:
				vertices[5] = marker[0];	vertices[6] = marker[1];
				vertices[10] = marker[2];	vertices[11] = marker[3];
				vertices[15] = marker[4];	vertices[16] = marker[5];
				vertices[0] = marker[6];	vertices[1] = marker[7];
				break;
			case MODE_CDAB:
				vertices[10] = marker[0];	vertices[11] = marker[1];
				vertices[15] = marker[2];	vertices[16] = marker[3];
				vertices[0] = marker[4];	vertices[1] = marker[5];
				vertices[5] = marker[6];	vertices[6] = marker[7];
				break;
			case MODE_DABC:
				vertices[15] = marker[0];	vertices[16] = marker[1];
				vertices[0] = marker[2];	vertices[1] = marker[3];
				vertices[5] = marker[4];	vertices[6] = marker[5];
				vertices[10] = marker[6];	vertices[11] = marker[7];
				break;
			case MODE_ADCB:
				vertices[0] = marker[0];	vertices[1] = marker[1];
				vertices[15] = marker[2];	vertices[16] = marker[3];
				vertices[10] = marker[4];	vertices[11] = marker[5];
				vertices[5] = marker[6];	vertices[6] = marker[7];
				break;
			case MODE_DCBA:
				vertices[15] = marker[0];	vertices[16] = marker[1];
				vertices[10] = marker[2];	vertices[11] = marker[3];
				vertices[5] = marker[4];	vertices[6] = marker[5];
				vertices[0] = marker[6];	vertices[1] = marker[7];
				break;
			case MODE_CBAD:
				vertices[10] = marker[0];	vertices[11] = marker[1];
				vertices[5] = marker[2];	vertices[6] = marker[3];
				vertices[0] = marker[4];	vertices[1] = marker[5];
				vertices[15] = marker[6];	vertices[16] = marker[7];
				break;
			case MODE_BADC:
				vertices[5] = marker[0];	vertices[6] = marker[1];
				vertices[0] = marker[2];	vertices[1] = marker[3];
				vertices[15] = marker[4];	vertices[16] = marker[5];
				vertices[10] = marker[6];	vertices[11] = marker[7];
				break;
			}


			ZQ_DImage<float> tex_bgra(tex_width, tex_height, 4);
			float*& tex_bgra_data = tex_bgra.data();
			const float*& tex_data = tex.data();
			const float*& tex_alpha_data = tex_alpha.data();
			if (tex_nChannels == 1)
			{
				for (int h = 0; h < tex_height; h++)
				{
					for (int w = 0; w < tex_width; w++)
					{
						tex_bgra_data[(h*tex_width + w) * 4 + 0] = tex_data[h*tex_width + w];
						tex_bgra_data[(h*tex_width + w) * 4 + 1] = tex_data[h*tex_width + w];
						tex_bgra_data[(h*tex_width + w) * 4 + 2] = tex_data[h*tex_width + w];
						tex_bgra_data[(h*tex_width + w) * 4 + 3] = tex_alpha_data[h*tex_width + w];
					}
				}
			}
			else if (tex_nChannels == 3)
			{
				for (int h = 0; h < tex_height; h++)
				{
					for (int w = 0; w < tex_width; w++)
					{
						memcpy(tex_bgra_data + (h*tex_width + w) * 4, tex_data + (h*tex_width + w) * 3, sizeof(float)* 3);
						tex_bgra_data[(h*tex_width + w) * 4 + 3] = tex_alpha_data[h*tex_width + w];
					}
				}
			}
			else
			{
				printf("invalid tex nChannels!\n");
				return false;
			}
			ZQ_TextureSampler<float> sampler;
			sampler.BindImage(tex_bgra, false);
			ZQ_CPURenderer2DWorkspace render2D(width, height);
			render2D.ClearDepthBuffer(1000);
			render2D.DisableDepthTest();
			render2D.EnableAlphaBlend();
			render2D.SetAlphaBlendMode(ZQ_CPURenderer3DWorkspace::ALPHABLEND_SRC_ALPHA_DST_ONE_MINUS_SRC);
			render2D.SetClip(1, 1000);
			render2D.BindSampler(&sampler);
			render2D.SetBackground(back);
			render2D.RenderIndexedTriangles(vertices, indices, 4, 2, ZQ_CPURenderer3DWorkspace::VERTEX_POSITION3_TEXCOORD2);
			const float*& buffer_data2D = render2D.GetColorBufferPtr();
			out_im.allocate(width, height, nChannels);
			float*& out_data = out_im.data();
			for (int h = 0; h < height; h++)
			{
				for (int w = 0; w < width; w++)
				{
					memcpy(out_data + (h*width + w)*nChannels, buffer_data2D + (h*width + w) * 4, sizeof(float)*nChannels);
				}
			}

			const float*& alpha_data = alpha.data();
			const float*& fore_data = fore.data();
			for (int i = 0; i < width*height; i++)
			{
				for (int c = 0; c < nChannels; c++)
				{
					out_data[i*nChannels + c] *= 1 - alpha_data[i];
					out_data[i*nChannels + c] += fore_data[i*nChannels + c] * alpha_data[i];
				}
			}
			return true;
		}
	};
	
}
#endif