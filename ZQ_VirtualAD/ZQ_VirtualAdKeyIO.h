#ifndef _ZQ_VIRTUAL_AD_KEY_IO_H_
#define _ZQ_VIRTUAL_AD_KEY_IO_H_
#pragma once

#include "ZQ_DoubleImage.h"
#include "ZQ_Vec2D.h"
#include <vector>

namespace ZQ
{
	class ZQ_VirtualAdKeyIO
	{
		/********** for polygon file **************/
	public:
		static bool LoadPolygonConfig(const char* file, std::vector<ZQ_Vec2D>& polygon)
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

		static bool SavePolygonConfig(const char* file, const std::vector<ZQ_Vec2D>& polygon)
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

		/********** for marker file **************/
	public:
		static bool LoadMarker(const char* file, double marker[8])
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
		static bool SaveMarker(const char* file, const double marker[8])
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

		/********** for H invH file **************/
	public:
		static bool Load_H_invH(const char* file, double H[9], double invH[9])
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

		static bool Save_H_invH(const char* file, const double H[9], const double invH[9])
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

		/**************** for key file (sift or surf)*****************/
	public:
		/*
		".key" for ascii
		".keyb" for binary
		*/
		static bool LoadSiftSurfKey(const char* file, int& num, int& dim, double*& coords, double*& vals)
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
				return LoadSiftSurfKey_ascii(file, num, dim, coords, vals);
			}
			else if (_strcmpi(file + i, ".keyb") == 0)
			{
				return LoadSiftSurfKey_binary(file, num, dim, coords, vals);
			}
			return false;
		}

		static bool LoadSiftSurfKey(const char* file, int& num, int& dim, float*& coords, float*& vals)
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
				return LoadSiftSurfKey_ascii(file, num, dim, coords, vals);
			}
			else if (_strcmpi(file + i, ".keyb") == 0)
			{
				return LoadSiftSurfKey_binary(file, num, dim, coords, vals);
			}
			return false;
		}

		/*
		".key" for ascii
		".keyb" for binary
		*/
		static bool SaveSiftSurfKey(const char* file, int num, int dim, const double* coords, const double* vals)
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
				return SaveSiftSurfKey_ascii(file, num, dim, coords, vals);
			}
			else if (_strcmpi(file + i, ".keyb") == 0)
			{
				return SaveSiftSurfKey_binary(file, num, dim, coords, vals);
			}
			return false;
		}

		static bool SaveSiftSurfKey(const char* file, int num, int dim, const float* coords, const float* vals)
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
				return SaveSiftSurfKey_ascii(file, num, dim, coords, vals);
			}
			else if (_strcmpi(file + i, ".keyb") == 0)
			{
				return SaveSiftSurfKey_binary(file, num, dim, coords, vals);
			}
			return false;
		}

		static bool LoadSiftSurfKey_ascii(const char* file, int& num, int& dim, double*& coords, double*& vals)
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

		static bool LoadSiftSurfKey_ascii(const char* file, int& num, int& dim, float*& coords, float*& vals)
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

		static bool LoadSiftSurfKey_binary(const char* file, int& num, int& dim, double*& coords, double*& vals)
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
				for (int d = 0; d < dim; d++)
					vals[p*dim + d] = tmp_vals[p*dim + d] / 65535.0;
			}
			delete[]tmp_coords;
			delete[]tmp_vals;
			return true;
		}

		static bool LoadSiftSurfKey_binary(const char* file, int& num, int& dim, float*& coords, float*& vals)
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
				for (int d = 0; d < dim; d++)
					vals[p*dim + d] = tmp_vals[p*dim + d] / 65535.0;
			}
			delete[]tmp_vals;
			return true;
		}


		static bool SaveSiftSurfKey_ascii(const char* file, int num, int dim, const double* coords, const double* vals)
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

		static bool SaveSiftSurfKey_ascii(const char* file, int num, int dim, const float* coords, const float* vals)
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

		static bool SaveSiftSurfKey_binary(const char* file, int num, int dim, const double* coords, const double* vals)
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
				tmp_vals[p] = __min(65535, __max(0, (vals[p] * 65535 + 0.5)));
			}
			fwrite(tmp_coords, sizeof(float), num * 2, out);
			fwrite(tmp_vals, sizeof(unsigned short), num * dim, out);
			fclose(out);

			delete[]tmp_coords;
			delete[]tmp_vals;
			return true;
		}

		static bool SaveSiftSurfKey_binary(const char* file, int num, int dim, const float* coords, const float* vals)
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

	public:
		template<class T>
		static bool FilterWithForegroundMask(int num, int dim, const T* coords, const T* vals, const ZQ_DImage<bool>& fore_mask,
			int& out_num, T*& out_coords, T*& out_vals)
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
				out_coords = new T[out_num * 2];
				out_vals = new T[out_num*dim];
				int k = 0;
				for (int i = 0; i < num; i++)
				{
					if (flag[i])
					{
						memcpy(out_coords + k * 2, coords + i * 2, sizeof(T)* 2);
						memcpy(out_vals + k*dim, vals + i*dim, sizeof(T)*dim);
						k++;
					}
				}
			}

			return true;
		}
	};
}

#endif