#ifndef Selection_h
#define Selection_h

class Selection{
public:
	int x;
	int y;
	int width;
	int height;
	bool valid = true;
	Selection(){
		valid = false;
	}
	Selection(int x1, int y1, int x2, int y2){
		if (x1 > x2){
			x = x2;
			width = x1 - x2;
		}
		else{
			x = x1;
			width = x2 - x1;
		}
		if (y1 > y2){
			y = y2;
			height = y1 - y2;
		}
		else{
			y = y1;
			height = y2 - y1;
		}
	}
	void expand(int ex, int ey){
		width += ex;
		height += ey;
	}
	Selection translate(int tx, int ty){
		return Selection(x + tx, y + ty, width, height);
	}
	bool contains(int px, int py){
		return px > x && px < x + width && py > y && py < y + height;
	}
};

#endif