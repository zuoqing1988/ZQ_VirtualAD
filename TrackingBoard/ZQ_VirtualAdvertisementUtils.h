#ifndef _ZQ_VIRTUAL_ADVERTISEMENT_UTILS_H_
#define _ZQ_VIRTUAL_ADVERTISEMENT_UTILS_H_
#pragma once

#include <vector>
#include "ZQ_Vec2D.h"
#include "ZQ_DoubleImage.h"
#include "ZQ_ImageIO.h"

namespace ZQ
{
	bool LoadPolygonConfig(const char* file, std::vector<ZQ_Vec2D>& polygon);
	bool SavePolygonConfig(const char* file, const std::vector<ZQ_Vec2D>& polygon);
	
	bool LoadMarker(const char* file, double marker[8]);
	bool SaveMarker(const char* file, const double marker[8]);
	bool Load_H_invH(const char* file, double H[9], double invH[9]);
	bool Save_H_invH(const char* file, const double H[9], const double invH[9]);

	/*
	".key" for ascii
	".keyb" for binary
	*/
	bool LoadSiftKey(const char* file, int& num, int& dim, double*& coords, double*& vals);
	bool LoadSiftKey(const char* file, int& num, int& dim, float*& coords, float*& vals);

	/*
	".key" for ascii
	".keyb" for binary
	*/
	bool SaveSiftKey(const char* file, int num, int dim, const double* coords, const double* vals);
	bool SaveSiftKey(const char* file, int num, int dim, const float* coords, const float* vals);

	bool LoadSiftKey_ascii(const char* file, int& num, int& dim, double*& coords, double*& vals);
	bool LoadSiftKey_ascii(const char* file, int& num, int& dim, float*& coords, float*& vals);
	bool LoadSiftKey_binary(const char* file, int& num, int& dim, double*& coords, double*& vals);
	bool LoadSiftKey_binary(const char* file, int& num, int& dim, float*& coords, float*& vals);
	bool SaveSiftKey_ascii(const char* file, int num, int dim, const double* coords, const double* vals);
	bool SaveSiftKey_ascii(const char* file, int num, int dim, const float* coords, const float* vals);
	bool SaveSiftKey_binary(const char* file, int num, int dim, const double* coords, const double* vals);
	bool SaveSiftKey_binary(const char* file, int num, int dim, const float* coords, const float* vals);

	bool FilterWithForegroundMask(int num, int dim, const double* coords, const double* vals, const ZQ_DImage<bool>& fore_mask,
		int& out_num, double*& out_coords, double*& out_vals);
	bool FilterWithForegroundMask(int num, int dim, const float* coords, const float* vals, const ZQ_DImage<bool>& fore_mask,
		int& out_num, float*& out_coords, float*& out_vals);

	void DrawPolygon(const char* fold, int fr, const std::vector<ZQ_Vec2D>& polygon, const char* suffix);
	void DrawPolygon(const ZQ_DImage<float>& im, ZQ_DImage<float>& out_im, const std::vector<ZQ_Vec2D>& polygon);
	void DrawMatch(const char* fold, int src_fr, int dst_fr, const std::vector<ZQ_Vec2D>& polygon, int match_num, const double* src_match_coords, const double* dst_match_coords, const bool* mask);
	void DrawMatch(const char* fold, int src_fr, int dst_fr, const std::vector<ZQ_Vec2D>& polygon, int match_num, const float* src_match_coords, const float* dst_match_coords, const bool* mask);
	void DrawMatch(const ZQ_DImage<float>& im1, const ZQ_DImage<float>& im2, ZQ_DImage<float>& out_im, const std::vector<ZQ_Vec2D>& polygon, int match_num, const double* src_match_coords, const double* dst_match_coords, const bool* mask);
	void DrawMatch(const ZQ_DImage<float>& im1, const ZQ_DImage<float>& im2, ZQ_DImage<float>& out_im, const std::vector<ZQ_Vec2D>& polygon, int match_num, const float* src_match_coords, const float* dst_match_coords, const bool* mask);

}
#endif