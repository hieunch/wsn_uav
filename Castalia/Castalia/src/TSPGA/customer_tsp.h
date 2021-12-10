#ifndef customer_tsp_H
#define customer_tsp_H

class customer_tsp{
public:
	customer_tsp(int id, double x, double y, int demand, int start, int end, int unload);
	
	int id;
	double x_pos, y_pos;
	int demand, start, end, unload;
};

#endif