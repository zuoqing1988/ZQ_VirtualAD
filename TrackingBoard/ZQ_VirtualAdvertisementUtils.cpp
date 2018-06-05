#include "ZQ_VirtualAdvertisementUtils.h"

namespace ZQ
{

	bool LoadPolygonConfig(const char* file, std::vector<ZQ_Vec2D>& polygon)
	{
		FILE* in = fopen(file, "r");
		if (in == 0)
			return false;

		int n;
		fscanf(in, "%d", &n);
		polygon.resize(n);
		for (int i = 0; i < n; i++)
		{
			double x, y;
			fscanf(in, "%lf%lf", &x, &y);
			polygon[i].x = x;
			polygon[i].y = y;
		}

		fclose(in);
		return true;
	}

	bool SavePolygonConfig(const char* file, const std::vector<ZQ_Vec2D>& polygon)
	{
		FILE* out = fopen(file, "w");
		if (out == 0)
			return false;

		int n = polygon.size();
		fprintf(out, "%d\n", n);

		for (int i = 0; i < n; i++)
		{
			fprintf(out, "%f %f\n", polygon[i].x, polygon[i].y);
		}

		fclose(out);
		return true;
	}

	bool LoadMarker(const char* file, double marker[8])
	{
		FILE* in = fopen(file, "r");
		if (in == 0)
			return false;

		int n;
		fscanf(in, "%d", &n);
		for (int i = 0; i < 4; i++)
		{
			double x, y;
			fscanf(in, "%lf%lf", &x, &y);
			marker[i * 2 + 0] = x;
			marker[i * 2 + 1] = y;
		}

		fclose(in);
		return true;
	}
	bool SaveMarker(const char* file, const double marker[8])
	{
		FILE* out = fopen(file, "w");
		if (out == 0)
			return false;
		fprintf(out, "%d\n", 4);
		for (int i = 0; i < 4; i++)
		{
			fprintf(out, "%f %f\n", marker[i * 2 + 0], marker[i * 2 + 1]);
		}

		fclose(out);
		return true;
	}

	bool Load_H_invH(const char* file, double H[9], double invH[9])
	{
		FILE* in = fopen(file, "rb");
		if (in == 0)
			return false;
		if (9 != fread(H, sizeof(double), 9, in))
		{
			fclose(in);
			return false;
		}
		if (9 != fread(invH, sizeof(double), 9, in))
		{
			fclose(in);
			return false;
		}
		fclose(in);
		return true;
	}

	bool Save_H_invH(const char* file, const double H[9], const double invH[9])
	{
		FILE* out = fopen(file, "wb");
		if (out == 0)
			return false;
		if (9 != fwrite(H, sizeof(double), 9, out))
		{
			fclose(out);
			return false;
		}
		if (9 != fwrite(invH, sizeof(double), 9, out))
		{
			fclose(out);
			return false;
		}
		fclose(out);
		return true;
	}

	bool LoadSiftKey(const char* file, int& num, int& dim, double*& coords, double*& vals)
	{
		if (file == 0)
			return false;
		int len = strlen(file);
		int i;
		for (i = len - 1; i >= 0; i--)
		{
			if (file[i] == '.')
				break;
		}
		if (i < 0)
			return false;
		if (_strcmpi(file + i, ".key") == 0)
		{
			return LoadSiftKey_ascii(file, num, dim, coords, vals);
		}
		else if (_strcmpi(file + i, ".keyb") == 0)
		{
			return LoadSiftKey_binary(file, num, dim, coords, vals);
		}
		return false;
	}
	bool LoadSiftKey(const char* file, int& num, int& dim, float*& coords, float*& vals)
	{
		if (file == 0)
			return false;
		int len = strlen(file);
		int i;
		for (i = len - 1; i >= 0; i--)
		{
			if (file[i] == '.')
				break;
		}
		if (i < 0)
			return false;
		if (_strcmpi(file + i, ".key") == 0)
		{
			return LoadSiftKey_ascii(file, num, dim, coords, vals);
		}
		else if (_strcmpi(file + i, ".keyb") == 0)
		{
			return LoadSiftKey_binary(file, num, dim, coords, vals);
		}
		return false;
	}

	bool SaveSiftKey(const char* file, int num, int dim, const double* coords, const double* vals)
	{
		if (file == 0)
			return false;
		int len = strlen(file);
		int i;
		for (i = len - 1; i >= 0; i--)
		{
			if (file[i] == '.')
				break;
		}
		if (i < 0)
			return false;
		if (_strcmpi(file + i, ".key") == 0)
		{
			return SaveSiftKey_ascii(file, num, dim, coords, vals);
		}
		else if (_strcmpi(file + i, ".keyb") == 0)
		{
			return SaveSiftKey_binary(file, num, dim, coords, vals);
		}
		return false;
	}

	bool SaveSiftKey(const char* file, int num, int dim, const float* coords, const float* vals)
	{
		if (file == 0)
			return false;
		int len = strlen(file);
		int i;
		for (i = len - 1; i >= 0; i--)
		{
			if (file[i] == '.')
				break;
		}
		if (i < 0)
			return false;
		if (_strcmpi(file + i, ".key") == 0)
		{
			return SaveSiftKey_ascii(file, num, dim, coords, vals);
		}
		else if (_strcmpi(file + i, ".keyb") == 0)
		{
			return SaveSiftKey_binary(file, num, dim, coords, vals);
		}
		return false;
	}

	bool LoadSiftKey_ascii(const char* file, int& num, int& dim, double*& coords, double*& vals)
	{
		FILE* in = fopen(file, "r");
		if (in == 0)
			return false;
		fscanf(in, "%d%d", &num, &dim);
		if (coords)
			delete[]coords;
		coords = new double[num * 2];
		if (vals)
			delete[]vals;
		vals = new double[num*dim];
		for (int p = 0; p < num; p++)
		{
			double tmp_val1, tmp_val2;
			fscanf(in, "%lf%lf%lf%lf", &coords[p * 2 + 1], &coords[p * 2 + 0], &tmp_val1, &tmp_val2);
			double len2 = 0;
			for (int d = 0; d < dim; d++)
			{
				fscanf(in, "%lf", &vals[p*dim + d]);
				len2 += vals[p*dim + d] * vals[p*dim + d];
			}
			if (len2 != 0)
			{
				double len = sqrt(len2);
				for (int d = 0; d < dim; d++)
					vals[p*dim + d] /= len;
			}
		}

		fclose(in);
		return true;
	}

	bool LoadSiftKey_ascii(const char* file, int& num, int& dim, float*& coords, float*& vals)
	{
		FILE* in = fopen(file, "r");
		if (in == 0)
			return false;
		fscanf(in, "%d%d", &num, &dim);
		if (coords)
			delete[]coords;
		coords = new float[num * 2];
		if (vals)
			delete[]vals;
		vals = new float[num*dim];
		for (int p = 0; p < num; p++)
		{
			double tmp_val1, tmp_val2;
			fscanf(in, "%f%f%f%f", &coords[p * 2 + 1], &coords[p * 2 + 0], &tmp_val1, &tmp_val2);
			double len2 = 0;
			for (int d = 0; d < dim; d++)
			{
				fscanf(in, "%f", &vals[p*dim + d]);
				len2 += vals[p*dim + d] * vals[p*dim + d];
			}
			if (len2 != 0)
			{
				double len = sqrt(len2);
				for (int d = 0; d < dim; d++)
					vals[p*dim + d] /= len;
			}
		}

		fclose(in);
		return true;
	}
	

	bool LoadSiftKey_binary(const char* file, int& num, int& dim, double*& coords, double*& vals)
	{
		FILE* in = fopen(file, "rb");
		if (in == 0)
			return false;
		fread(&num, sizeof(int), 1, in);
		fread(&dim, sizeof(int), 1, in);

		if (coords)
			delete[]coords;
		coords = new double[num * 2];
		if (vals)
			delete[]vals;
		vals = new double[num*dim];
		
		float* tmp_coords = new float[num * 2];
		unsigned short* tmp_vals = new unsigned short[num*dim];
		fread(tmp_coords, sizeof(float), 2 * num, in);
		fread(tmp_vals, sizeof(unsigned short), dim * num, in);
		fclose(in);
		for (int p = 0; p < num * 2; p++)
		{
			coords[p] = tmp_coords[p];
		}
		for (int p = 0; p < num; p++)
		{
			/*double len2 = 0;
			for (int d = 0; d < dim; d++)
			{
				
				len2 += (double)tmp_vals[p*dim + d] * tmp_vals[p*dim + d];
			}
			if (len2 != 0)
			{
				double len = sqrt(len2);
				for (int d = 0; d < dim; d++)
					vals[p*dim + d] = (double)tmp_vals[p*dim + d] / len;
			}
			else*/
			//{
			for (int d = 0; d < dim; d++)
				vals[p*dim + d] = tmp_vals[p*dim + d] / 65535.0;
			//}
		}
		delete[]tmp_coords;
		delete[]tmp_vals;
		return true;
	}

	bool LoadSiftKey_binary(const char* file, int& num, int& dim, float*& coords, float*& vals)
	{
		FILE* in = fopen(file, "rb");
		if (in == 0)
			return false;
		fread(&num, sizeof(int), 1, in);
		fread(&dim, sizeof(int), 1, in);

		if (coords)
			delete[]coords;
		coords = new float[num * 2];
		if (vals)
			delete[]vals;
		vals = new float[num*dim];

		unsigned short* tmp_vals = new unsigned short[num*dim];
		fread(coords, sizeof(float), 2 * num, in);
		fread(tmp_vals, sizeof(unsigned short), dim * num, in);
		fclose(in);
		
		for (int p = 0; p < num; p++)
		{
			/*double len2 = 0;
			for (int d = 0; d < dim; d++)
			{

			len2 += (double)tmp_vals[p*dim + d] * tmp_vals[p*dim + d];
			}
			if (len2 != 0)
			{
			double len = sqrt(len2);
			for (int d = 0; d < dim; d++)
			vals[p*dim + d] = (double)tmp_vals[p*dim + d] / len;
			}
			else*/
			//{
			for (int d = 0; d < dim; d++)
				vals[p*dim + d] = tmp_vals[p*dim + d] / 65535.0;
			//}
		}
		delete[]tmp_vals;
		return true;
	}

	
	bool SaveSiftKey_ascii(const char* file, int num, int dim, const double* coords, const double* vals)
	{
		FILE* out = fopen(file, "w");
		if (out == 0)
			return false;
		fprintf(out, "%d %d\n", num, dim);
		for (int p = 0; p < num; p++)
		{
			double tmp_val1 = 0, tmp_val2 = 0;
			fprintf(out, "%.3f %.3f %.3f %.3f\n", coords[p * 2 + 1], coords[p * 2 + 0], tmp_val1, tmp_val2);
			for (int d = 0; d < dim; d++)
			{
				fprintf(out, "%.3f ", vals[p*dim + d]);

			}
			fprintf(out, "\n");
		}
		fclose(out);
		return true;
	}

	bool SaveSiftKey_ascii(const char* file, int num, int dim, const float* coords, const float* vals)
	{
		FILE* out = fopen(file, "w");
		if (out == 0)
			return false;
		fprintf(out, "%d %d\n", num, dim);
		for (int p = 0; p < num; p++)
		{
			float tmp_val1 = 0, tmp_val2 = 0;
			fprintf(out, "%.3f %.3f %.3f %.3f\n", coords[p * 2 + 1], coords[p * 2 + 0], tmp_val1, tmp_val2);
			for (int d = 0; d < dim; d++)
			{
				fprintf(out, "%.3f ", vals[p*dim + d]);

			}
			fprintf(out, "\n");
		}
		fclose(out);
		return true;
	}

	bool SaveSiftKey_binary(const char* file, int num, int dim, const double* coords, const double* vals)
	{
		FILE* out = fopen(file, "wb");
		if (out == 0)
			return false;
		fwrite(&num, sizeof(int), 1, out);
		fwrite(&dim, sizeof(int), 1, out);
		float* tmp_coords = new float[num * 2];
		unsigned short* tmp_vals = new unsigned short[num*dim];
		for (int p = 0; p < num * 2; p++)
		{
			tmp_coords[p] = coords[p];
		}
		for (int p = 0; p < num*dim; p++)
		{
			tmp_vals[p] = __min(65535,__max(0,(vals[p]*65535+0.5)));
		}
		fwrite(tmp_coords, sizeof(float), num * 2, out);
		fwrite(tmp_vals, sizeof(unsigned short), num * dim, out);
		fclose(out);
		
		delete[]tmp_coords;
		delete[]tmp_vals;
		return true;
	}

	bool SaveSiftKey_binary(const char* file, int num, int dim, const float* coords, const float* vals)
	{
		FILE* out = fopen(file, "wb");
		if (out == 0)
			return false;
		fwrite(&num, sizeof(int), 1, out);
		fwrite(&dim, sizeof(int), 1, out);
		
		unsigned short* tmp_vals = new unsigned short[num*dim];
		for (int p = 0; p < num*dim; p++)
		{
			tmp_vals[p] = __min(65535, __max(0, (vals[p] * 65535 + 0.5)));
		}
		fwrite(coords, sizeof(float), num * 2, out);
		fwrite(tmp_vals, sizeof(unsigned short), num * dim, out);
		fclose(out);
		delete[]tmp_vals;
		return true;
	}

	bool FilterWithForegroundMask(int num, int dim, const double* coords, const double* vals, const ZQ_DImage<bool>& fore_mask,
		int& out_num, double*& out_coords, double*& out_vals)
	{

		if (out_coords)
		{
			delete[]out_coords;
			out_coords = 0;
		}
		if (out_vals)
		{
			delete[]out_vals;
			out_vals = 0;
		}
		if (num == 0)
		{
			out_num = 0;
			return true;
		}

		int width = fore_mask.width();
		int height = fore_mask.height();
		bool* flag = new bool[num];
		out_num = 0;
		const bool*& mask_data = fore_mask.data();
		for (int i = 0; i < num; i++)
		{
			int ix = coords[i * 2 + 0];
			int iy = coords[i * 2 + 1];
			if (ix >= 0 && ix < width && iy >= 0 && iy < height && !mask_data[iy*width + ix])
			{
				flag[i] = true;
				out_num++;
			}
			else
				flag[i] = false;
		}
		if (out_num > 0)
		{
			out_coords = new double[out_num * 2];
			out_vals = new double[out_num*dim];
			int k = 0;
			for (int i = 0; i < num; i++)
			{
				if (flag[i])
				{
					memcpy(out_coords + k * 2, coords + i * 2, sizeof(double)* 2);
					memcpy(out_vals + k*dim, vals + i*dim, sizeof(double)*dim);
					k++;
				}
			}
		}

		return true;
	}

	bool FilterWithForegroundMask(int num, int dim, const float* coords, const float* vals, const ZQ_DImage<bool>& fore_mask,
		int& out_num, float*& out_coords, float*& out_vals)
	{

		if (out_coords)
		{
			delete[]out_coords;
			out_coords = 0;
		}
		if (out_vals)
		{
			delete[]out_vals;
			out_vals = 0;
		}
		if (num == 0)
		{
			out_num = 0;
			return true;
		}

		int width = fore_mask.width();
		int height = fore_mask.height();
		bool* flag = new bool[num];
		out_num = 0;
		const bool*& mask_data = fore_mask.data();
		for (int i = 0; i < num; i++)
		{
			int ix = coords[i * 2 + 0];
			int iy = coords[i * 2 + 1];
			if (ix >= 0 && ix < width && iy >= 0 && iy < height && !mask_data[iy*width + ix])
			{
				flag[i] = true;
				out_num++;
			}
			else
				flag[i] = false;
		}
		if (out_num > 0)
		{
			out_coords = new float[out_num * 2];
			out_vals = new float[out_num*dim];
			int k = 0;
			for (int i = 0; i < num; i++)
			{
				if (flag[i])
				{
					memcpy(out_coords + k * 2, coords + i * 2, sizeof(float)* 2);
					memcpy(out_vals + k*dim, vals + i*dim, sizeof(float)*dim);
					k++;
				}
			}
		}

		return true;
	}

	void DrawPolygon(const char* fold, int fr, const std::vector<ZQ_Vec2D>& polygon, const char* suffix)
	{
		//CvScalar rect_color = cvScalar(0, 0, 255);
		CvScalar poly_color = cvScalar(0, 255, 0);

		char file[200];
		sprintf(file, "%s\\images\\%d.jpg", fold, fr);
		ZQ_DImage<float> im, out_im;
		if (!ZQ_ImageIO::loadImage(im, file, 1))
		{
			printf("failed to load %s\n", file);
			return;
		}
		DrawPolygon(im, out_im, polygon);
		sprintf(file, "%s\\track\\%d%s.jpg", fold, fr, suffix);
		if (!ZQ_ImageIO::saveImage(out_im, file))
		{
			printf("failed to save %s\n", file);
			return;
		}
	}

	void DrawPolygon(const ZQ_DImage<float>& im, ZQ_DImage<float>& out_im, const std::vector<ZQ_Vec2D>& polygon)
	{
		//CvScalar rect_color = cvScalar(0, 0, 255);
		CvScalar poly_color = cvScalar(0, 255, 0);

		int width = im.width();
		int height = im.height();
		int nChannels = im.nchannels();
		out_im.allocate(width, height, nChannels);
		IplImage* ori = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, nChannels);
		if (ori == 0)
			return;

		const float*& im_data = im.data();
		float*& out_im_data = out_im.data();
		for (int h = 0; h < height; h++)
		{
			for (int w = 0; w < width; w++)
			{
				CvScalar sca;
				for (int c = 0; c < nChannels; c++)
				{
					sca.val[c] = im_data[(h*width + w)*nChannels + c] * 255;
				}
				cvSet2D(ori, h, w, sca);
			}
		}

		int size = polygon.size();
		if (size >= 2)
		{
			for (int i = 0; i < size; i++)
			{
				CvPoint pt1 = cvPoint(polygon[i].x, polygon[i].y);
				CvPoint pt2 = cvPoint(polygon[(i + 1) % size].x, polygon[(i + 1) % size].y);
				cvLine(ori, pt1, pt2, poly_color);
			}
		}
		for (int h = 0; h < height; h++)
		{
			for (int w = 0; w < width; w++)
			{
				CvScalar sca = cvGet2D(ori, h, w);
				for (int c = 0; c < nChannels; c++)
				{
					out_im_data[(h*width + w)*nChannels + c] = sca.val[c] / 255;
				}
			}
		}
		cvReleaseImage(&ori);
	}

	void DrawMatch(const char* fold, int src_fr, int dst_fr, const std::vector<ZQ_Vec2D>& polygon,
		int match_num, const double* src_match_coords, const double* dst_match_coords, const bool* mask)
	{
		//CvScalar rect_color = cvScalar(0, 0, 255);
		CvScalar poly_color = cvScalar(0, 255, 0);
		CvScalar match_line_color = cvScalar(255, 0, 0);
		CvScalar match_line_color2 = cvScalar(255, 255, 0);

		ZQ_DImage<float> im1, im2, out_im;
		char file[200];
		sprintf(file, "%s\\images\\%d.jpg", fold, src_fr);
		if (!ZQ_ImageIO::loadImage(im1, file, 1))
		{
			printf("failed to load %s\n", file);
			return;
		}
		sprintf(file, "%s\\images\\%d.jpg", fold, dst_fr);
		if (!ZQ_ImageIO::loadImage(im2, file, 1))
		{
			printf("failed to load %s\n", file);
			return;
		}

		DrawMatch(im1, im2, out_im, polygon, match_num, src_match_coords, dst_match_coords, mask);
		sprintf(file, "%s\\match\\%d_%d.jpg", fold, src_fr, dst_fr);
		if (!ZQ_ImageIO::saveImage(out_im, file))
		{
			printf("failed to save %s\n", file);
			return;
		}
	}

	void DrawMatch(const char* fold, int src_fr, int dst_fr, const std::vector<ZQ_Vec2D>& polygon,
		int match_num, const float* src_match_coords, const float* dst_match_coords, const bool* mask)
	{
		//CvScalar rect_color = cvScalar(0, 0, 255);
		CvScalar poly_color = cvScalar(0, 255, 0);
		CvScalar match_line_color = cvScalar(255, 0, 0);
		CvScalar match_line_color2 = cvScalar(255, 255, 0);

		ZQ_DImage<float> im1, im2, out_im;
		char file[200];
		sprintf(file, "%s\\images\\%d.jpg", fold, src_fr);
		if (!ZQ_ImageIO::loadImage(im1, file, 1))
		{
			printf("failed to load %s\n", file);
			return;
		}
		sprintf(file, "%s\\images\\%d.jpg", fold, dst_fr);
		if (!ZQ_ImageIO::loadImage(im2, file, 1))
		{
			printf("failed to load %s\n", file);
			return;
		}

		DrawMatch(im1, im2, out_im, polygon, match_num, src_match_coords, dst_match_coords, mask);
		sprintf(file, "%s\\match\\%d_%d.jpg", fold, src_fr, dst_fr);
		if (!ZQ_ImageIO::saveImage(out_im, file))
		{
			printf("failed to save %s\n", file);
			return;
		}
	}


	void DrawMatch(const ZQ_DImage<float>& im1, const ZQ_DImage<float>& im2, ZQ_DImage<float>& out_im, const std::vector<ZQ_Vec2D>& polygon, int match_num, const double* src_match_coords, const double* dst_match_coords, const bool* mask)
	{
		//CvScalar rect_color = cvScalar(0, 0, 255);
		CvScalar poly_color = cvScalar(0, 255, 0);
		CvScalar match_line_color = cvScalar(255, 0, 0);
		CvScalar match_line_color2 = cvScalar(255, 255, 0);

		int width1 = im1.width();
		int height1 = im1.height();
		int nChannels1 = im1.nchannels();
		int width2 = im2.width();
		int height2 = im2.height();
		int nChannels2 = im2.nchannels();
		out_im.clear();
		if (nChannels1 != nChannels2 || height1 != height2)
			return;
		const float*& im1_data = im1.data();
		const float*& im2_data = im2.data();

		IplImage* ori = cvCreateImage(cvSize(width1 + width2, height1), IPL_DEPTH_8U, nChannels1);
		for (int h = 0; h < height1; h++)
		{
			for (int w = 0; w < width1; w++)
			{
				CvScalar sca;
				for (int c = 0; c < nChannels1; c++)
				{
					sca.val[c] = im1_data[(h*width1 + w)*nChannels1 + c] * 255;
				}
				cvSet2D(ori, h, w, sca);
			}
			for (int w = 0; w < width2; w++)
			{
				CvScalar sca;
				for (int c = 0; c < nChannels1; c++)
				{
					sca.val[c] = im2_data[(h*width2 + w)*nChannels1 + c] * 255;
				}
				cvSet2D(ori, h, w + width1, sca);
			}
		}

		int size = polygon.size();
		if (size >= 2)
		{
			for (int i = 0; i < size; i++)
			{
				CvPoint pt1 = cvPoint(width1 + polygon[i].x, polygon[i].y);
				CvPoint pt2 = cvPoint(width1 + polygon[(i + 1) % size].x, polygon[(i + 1) % size].y);
				cvLine(ori, pt1, pt2, poly_color);
			}
		}
		for (int i = 0; i < match_num; i++)
		{
			CvPoint pt1 = cvPoint(src_match_coords[i * 2 + 0], src_match_coords[i * 2 + 1]);
			CvPoint pt2 = cvPoint(width1 + dst_match_coords[i * 2 + 0], dst_match_coords[i * 2 + 1]);
			if (mask[i])
				cvLine(ori, pt1, pt2, match_line_color, 3);
			else
				cvLine(ori, pt1, pt2, match_line_color2, 1);
		}

		out_im.allocate(width1 + width2, height1, nChannels1);
		float*& out_im_data = out_im.data();
		for (int h = 0; h < height1; h++)
		{
			for (int w = 0; w < width1 + width2; w++)
			{
				CvScalar sca = cvGet2D(ori, h, w);
				for (int c = 0; c < nChannels1; c++)
				{
					out_im_data[(h*(width1 + width2) + w)*nChannels1 + c] = sca.val[c] / 255;
				}
			}
		}
		cvReleaseImage(&ori);
	}
	void DrawMatch(const ZQ_DImage<float>& im1, const ZQ_DImage<float>& im2, ZQ_DImage<float>& out_im, const std::vector<ZQ_Vec2D>& polygon, int match_num, const float* src_match_coords, const float* dst_match_coords, const bool* mask)
	{
		//CvScalar rect_color = cvScalar(0, 0, 255);
		CvScalar poly_color = cvScalar(0, 255, 0);
		CvScalar match_line_color = cvScalar(255, 0, 0);
		CvScalar match_line_color2 = cvScalar(255, 255, 0);

		int width1 = im1.width();
		int height1 = im1.height();
		int nChannels1 = im1.nchannels();
		int width2 = im2.width();
		int height2 = im2.height();
		int nChannels2 = im2.nchannels();
		out_im.clear();
		if (nChannels1 != nChannels2 || height1 != height2)
			return;
		const float*& im1_data = im1.data();
		const float*& im2_data = im2.data();

		IplImage* ori = cvCreateImage(cvSize(width1 + width2, height1), IPL_DEPTH_8U, nChannels1);
		for (int h = 0; h < height1; h++)
		{
			for (int w = 0; w < width1; w++)
			{
				CvScalar sca;
				for (int c = 0; c < nChannels1; c++)
				{
					sca.val[c] = im1_data[(h*width1 + w)*nChannels1 + c] * 255;
				}
				cvSet2D(ori, h, w, sca);
			}
			for (int w = 0; w < width2; w++)
			{
				CvScalar sca;
				for (int c = 0; c < nChannels1; c++)
				{
					sca.val[c] = im2_data[(h*width2 + w)*nChannels1 + c] * 255;
				}
				cvSet2D(ori, h, w + width1, sca);
			}
		}

		int size = polygon.size();
		if (size >= 2)
		{
			for (int i = 0; i < size; i++)
			{
				CvPoint pt1 = cvPoint(width1 + polygon[i].x, polygon[i].y);
				CvPoint pt2 = cvPoint(width1 + polygon[(i + 1) % size].x, polygon[(i + 1) % size].y);
				cvLine(ori, pt1, pt2, poly_color);
			}
		}
		for (int i = 0; i < match_num; i++)
		{
			CvPoint pt1 = cvPoint(src_match_coords[i * 2 + 0], src_match_coords[i * 2 + 1]);
			CvPoint pt2 = cvPoint(width1 + dst_match_coords[i * 2 + 0], dst_match_coords[i * 2 + 1]);
			if (mask[i])
				cvLine(ori, pt1, pt2, match_line_color, 3);
			else
				cvLine(ori, pt1, pt2, match_line_color2, 1);
		}

		out_im.allocate(width1 + width2, height1, nChannels1);
		float*& out_im_data = out_im.data();
		for (int h = 0; h < height1; h++)
		{
			for (int w = 0; w < width1 + width2; w++)
			{
				CvScalar sca = cvGet2D(ori, h, w);
				for (int c = 0; c < nChannels1; c++)
				{
					out_im_data[(h*(width1 + width2) + w)*nChannels1 + c] = sca.val[c] / 255;
				}
			}
		}
		cvReleaseImage(&ori);
	}
}