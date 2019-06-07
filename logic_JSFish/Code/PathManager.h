#ifndef __PATH_MANAGER_H__
#define __PATH_MANAGER_H__

///////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"

///////////////////////////////////////////////////////////////////////////////////////////
#include "ActionCustom.h"

///////////////////////////////////////////////////////////////////////////////////////////
class Path_Manager
{
public:
	Path_Manager();
	~Path_Manager();

public:
	void initialise();
	bool initialise_paths(const std::string &directory);
	Move_Points &get_paths(uint16_t path_id, uint8_t path_type);

	bool initialise_scene_shape(const std::string &directory);

	int get_scene_shape_count(int direction) const;
	Move_Points &get_scene_shape(int index, int direction);


private:
	std::vector<Move_Points> small_paths_;
    std::vector<Move_Points> big_paths_;
    std::vector<Move_Points> huge_paths_;
    std::vector<Move_Points> special_paths_;
    std::vector<Move_Points> scene_paths_;

    std::vector<Move_Points> scene_shape_left_;
    std::vector<Move_Points> scene_shape_right_;

	std::string work_dir_;

	bool load_success_;
};

///////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////

#endif