#include "stdafx.h"
#include "Exception.h"
#include "PathManager.h"
/*#include "utility.h"*/
#include "../../NetModule/Utility.h"
///////////////////////////////////////////////////////////////////////////
Path_Manager::Path_Manager()
{
	load_success_ = false;
}

Path_Manager::~Path_Manager()
{
	small_paths_.clear();
    big_paths_.clear();
    huge_paths_.clear();
    special_paths_.clear();
    scene_paths_.clear();
}

void Path_Manager::initialise()
{
	if (load_success_)
		return;
	
	//char szPath[MAX_PATH]="";
	//GetCurrentDirectory(sizeof(szPath),szPath);
	std::string strCurDir = Utility::GetCurrentDirectory();
	if(strCurDir.size() >= 512)
	{
		return;
	}

	char path_server[512];
	//_snprintf(path_server,260,"%s\\config",szPath);
	snprintf(path_server, 512, "%s/script/js_config", strCurDir.c_str());
	work_dir_ = path_server;
	initialise_paths("");
	initialise_scene_shape("");
	load_success_ = true;
}
bool Path_Manager::initialise_paths(const std::string &directory)
{
	small_paths_.clear();
	big_paths_.clear();
	huge_paths_.clear();
	special_paths_.clear();
	scene_paths_.clear();

	int x,y,staff;
	float angle;
	char path[100];
	FILE *pf;
	std::ostringstream ostr;

	char file_name[260];
	std::string file_path;

	for (int i=0; i < MAX_SMALL_PATH; i++)
	{
		Move_Points move_points;

		//_snprintf(file_name,260,"\\path\\small\\%d.dat",i);
		snprintf(file_name, 260, "/path/small/%d.dat",i);

		file_path = work_dir_ + file_name;
		pf = fopen(file_path.c_str(), "rb");

		if (pf == 0)
		{
			return false;
		}

		fgets(path, 100, pf);

		while(fgets(path, 100, pf))
		{
			std::sscanf(path, "(%d,%d,%f,%d)", &x, &y, &angle, &staff);
			move_points.push_back(Move_Point(Point(x, y), angle));
		}

		small_paths_.push_back(move_points);

		fclose(pf);

	}

	for (int i=0; i<MAX_BIG_PATH; i++)
	{
		Move_Points move_points;

		//_snprintf(file_name,260,"\\path\\big\\%d.dat",i);
		snprintf(file_name,260,"/path/big/%d.dat",i);
		file_path = work_dir_ + file_name;

		pf = fopen(file_path.c_str(), "rb");
		if (pf == 0)
		{
			return false;
		}

		fgets(path, 100, pf);

		while(fgets(path, 100, pf))
		{
			std::sscanf(path, "(%d,%d,%f,%d)", &x, &y, &angle, &staff);
			move_points.push_back(Move_Point(Point(x, y), angle));
		}

		big_paths_.push_back(move_points);

		fclose(pf);

	}


	for (int i=0; i<MAX_HUGE_PATH; i++)
	{
		Move_Points move_points;

		//_snprintf(file_name,260,"\\path\\huge\\%d.dat",i);
		snprintf(file_name,260,"/path/huge/%d.dat",i);
		file_path = work_dir_ + file_name;

		pf = fopen(file_path.c_str(), "rb");
		if (pf == 0)
		{
			return false;
		}

		fgets(path, 100, pf);

		while(fgets(path, 100, pf))
		{
			std::sscanf(path, "(%d,%d,%f,%d)", &x, &y, &angle, &staff);
			move_points.push_back(Move_Point(Point(x, y), angle));
		}

		huge_paths_.push_back(move_points);

		fclose(pf);

	}

	for (int i=0; i < MAX_SPECIAL_PATH; i++)
	{
		Move_Points move_points;

		//_snprintf(file_name,260,"\\path\\special\\%d.dat",i);
		snprintf(file_name,260,"/path/special/%d.dat",i);
		file_path = work_dir_ + file_name;


		pf = fopen(file_path.c_str(), "rb");
		if (pf == 0)
		{
			return false;
		}

		fgets(path, 100, pf);

		while(fgets(path, 100, pf))
		{
			std::sscanf(path, "(%d,%d,%f,%d)", &x, &y, &angle, &staff);
			move_points.push_back(Move_Point(Point(x, y), angle));
		}

		special_paths_.push_back(move_points);

		fclose(pf);

	}

	for (int i=0; i < MAX_SCENE_PATH; i++)
	{
		Move_Points move_points;

		//_snprintf(file_name,260,"\\path\\scene\\%d.dat",i);
		snprintf(file_name,260,"/path/scene/%d.dat",i);
		file_path = work_dir_ + file_name;

		pf = fopen(file_path.c_str(), "rb");
		if (pf == 0)
		{
			return false;
		}

		fgets(path, 100, pf);

		while(fgets(path, 100, pf))
		{
			std::sscanf(path, "(%d,%d,%f,%d)", &x, &y, &angle, &staff);
			move_points.push_back(Move_Point(Point(x, y), angle));
		}

		scene_paths_.push_back(move_points);

		fclose(pf);

	}

	return true;
}
Move_Points &Path_Manager::get_paths(uint16_t path_id, uint8_t path_type)
{
	if (path_type == PATH_TYPE_SMALL)
	{
		if (path_id >= MAX_SMALL_PATH)
			throw_normal_error("get_paths small path_id too big");

		return small_paths_[path_id];
	}
	else if (path_type == PATH_TYPE_BIG)
	{
		if (path_id >= MAX_BIG_PATH )
			throw_normal_error("get_paths big path_id too big");

		return big_paths_[path_id];
	}
	else if (path_type == PATH_TYPE_HUGE)
	{
		if (path_id >= MAX_HUGE_PATH )
			throw_normal_error("get_paths huge path_id too big");

		return huge_paths_[path_id];
	}
	else if (path_type == PATH_TYPE_SPECIAL)
	{	
		if (path_id >= MAX_SPECIAL_PATH )
			throw_normal_error("get_paths special path_id too big");

		return special_paths_[path_id];
	}
	else
	{
		if (path_id >= MAX_SCENE_PATH )
			throw_normal_error("get_paths scene path_id too big");

		return scene_paths_[path_id];
	}

}

bool Path_Manager::initialise_scene_shape(const std::string &directory)
{
	scene_shape_left_.clear();
	scene_shape_right_.clear();

	int x,y,staff;
//	float angle=0;
	char path[100] = {0};
	FILE *pf;
	std::ostringstream ostr;

	char file_name[260];
	std::string file_path;

	for (int i=0; i < MAX_SCENE_SHAPE; i++)
	{
		Move_Points move_points;

		//_snprintf(file_name,260,"\\sceneshape\\%d.dat",i);
		snprintf(file_name,260,"/sceneshape/%d.dat",i);
		file_path = work_dir_ + file_name;

		pf = fopen(file_path.c_str(), "rb");

		if (pf == 0)
		{
			return false;
		}

		fgets(path, 100, pf);
		std::sscanf(path, "%d", &x);

		if (x == 0)
		{
			while(fgets(path, 100, pf))
			{
				std::sscanf(path, "(%d, %d, %d)", &x, &y, &staff);
				move_points.push_back(Move_Point(Point(x, y), staff));
			}

			scene_shape_left_.push_back(move_points);
		}
		else
		{
			while(fgets(path, 100, pf))
			{
				std::sscanf(path, "(%d, %d, %d)", &x, &y, &staff);
				move_points.push_back(Move_Point(Point(x, y), staff));
			}

			scene_shape_right_.push_back(move_points);
		}

		fclose(pf);

	}

	return true;
}

int Path_Manager::get_scene_shape_count(int direction) const
{
	if (direction == 0)
		return scene_shape_left_.size();
	else
		return scene_shape_right_.size();
}

Move_Points &Path_Manager::get_scene_shape(int index, int direction)
{
	if (direction == 0)
		return scene_shape_left_[index];
	else
		return scene_shape_right_[index];
}

///////////////////////////////////////////////////////////////////////////
