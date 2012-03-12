#ifndef pointStack_
#define pointStack_

struct point
{
	int x,y;
};

//  Point stack for floodFill algorithm
//  Implented as a linked list
struct pointStackElement
{
	point elementData;
	pointStackElement* nextElement;
};

//  Pop/Push. Head set to zero if empty.
point pointStackPop(point** head);
void pointStackPush(point* head, point pt);

#endif