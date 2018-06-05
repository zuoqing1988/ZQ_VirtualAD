#ifndef _ZQ_SHAPE_AND_SCENE_CUT_H_
#define _ZQ_SHAPE_AND_SCENE_CUT_H_
#pragma once

#include <vector>
#include <opencv2\opencv.hpp>

namespace ZQ
{
	class ZQ_SceneCut
	{
		friend class ZQ_DetectShapeAndSceneCut;
	public:
		class Shape
		{
		public:
			int frame_id;
			float area_size;
			int feature_num;
			std::vector<cv::Point> poly;
			Shape()
			{
				frame_id = -1;
				area_size = -1;
				feature_num = -1;
			}
			void operator=(const Shape& other)
			{
				this->frame_id = other.frame_id;
				this->area_size = other.area_size;
				this->feature_num = other.feature_num;
				this->poly = other.poly;
			}

			void Swap(Shape& other)
			{
				int tmp_id = frame_id;
				frame_id = other.frame_id;
				other.frame_id = tmp_id;
				float tmp_area = area_size;
				area_size = other.area_size;
				other.area_size = tmp_area;
				int tmp_n = feature_num;
				feature_num = other.feature_num;
				other.feature_num = tmp_n;
				poly.swap(other.poly);
			}

			void ExportToFile(FILE* out) const
			{
				fprintf(out, "\t\t%d %d %.f %d \n", frame_id, feature_num, area_size, poly.size());
				if (poly.size() > 0)
					fprintf(out, "\t\t");
				for (int i = 0; i < poly.size(); i++)
				{
					fprintf(out, "%d %d ", poly[i].x, poly[i].y);
				}
				if (poly.size() > 0)
					fprintf(out,"\n");
			}
			void ImportFromFile(FILE* in)
			{
				int size;
				fscanf(in, "%d%d%f%d", &frame_id, &feature_num, &area_size, &size);
				poly.resize(size);
				for (int i = 0; i < size; i++)
				{
					fscanf(in, "%d%d", &poly[i].x, &poly[i].y);
				}
			}
		};
	protected:
		int start_frame_id;
		int end_frame_id;
		bool is_static_scene;
		std::vector<Shape> shapes;

	public:
		const int GetShapeNum() const {	return shapes.size();}
		const int GetStartFrameID() const{ return start_frame_id; }
		const int GetEndFrameID() const { return end_frame_id; }
		const Shape& GetShape(int i) const{ return shapes[i]; }
		const bool IsStaticScene() const{ return is_static_scene; }
		
		//be careful to use this
		std::vector<Shape>& GetShapes() { return shapes; }

		void ExportToFile(FILE* out) const
		{
			fprintf(out, "\t%d %d %d %d \n", start_frame_id, end_frame_id, shapes.size(),is_static_scene ? 1 : 0);
			for (int i = 0; i < shapes.size(); i++)
			{
				shapes[i].ExportToFile(out);
			}
		}

		void ImportFromFile(FILE* in)
		{
			int size,bval;
			fscanf(in, "%d%d%d%d", &start_frame_id, &end_frame_id, &size,&bval);
			shapes.resize(size);
			is_static_scene = bval;
			for (int i = 0; i < size; i++)
			{
				shapes[i].ImportFromFile(in);
			}
		}
		void SortDec_FeatNum_AreaSize()
		{
			int num = shapes.size();
			for (int pass = 0; pass < num - 1; pass++)
			{
				for (int i = 0; i < num - 1; i++)
				{
					int cur_feat_num = shapes[i].feature_num;
					int next_feat_num = shapes[i + 1].feature_num;
					if (cur_feat_num < next_feat_num)
					{
						shapes[i].Swap(shapes[i + 1]);
					}
					else if (cur_feat_num == next_feat_num)
					{
						if (shapes[i].area_size < shapes[i + 1].area_size)
						{
							shapes[i].Swap(shapes[i + 1]);
						}
					}
				}
			}
		}
		
	};

	class ZQ_SceneCutList
	{
	public:
		std::vector<ZQ_SceneCut> scene_cuts;

		bool ExportToFile(const char* file) const
		{
			FILE* out = fopen(file, "w");
			if (out == 0)
				return false;
			fprintf(out, "%d \n", scene_cuts.size());
			for (int i = 0; i < scene_cuts.size(); i++)
				scene_cuts[i].ExportToFile(out);
			fclose(out);
			return true;
		}

		bool ImportFile(const char* file)
		{
			FILE* in = fopen(file, "r");
			if (in == 0)
				return false;
			int size;
			fscanf(in, "%d", &size);
			scene_cuts.resize(size);
			for (int i = 0; i < size; i++)
			{
				scene_cuts[i].ImportFromFile(in);
			}
			return true;
		}

		void SortDec_FeatNum_AreaSize()
		{
			int num = scene_cuts.size();
			for (int i = 0; i < num; i++)
				scene_cuts[i].SortDec_FeatNum_AreaSize();
		}
	};
}

#endif