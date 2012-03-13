#include "stdafx.h"
#include <stdlib.h>
#include "pointStack.h"


point pointStackPop(pointStackElement** head)
{
	if(*head != 0)
	{
		point temp = (*head)->elementData;			//  Grab data
		*head = (*head)->nextElement;				//  Update head
		free(*head);								//  Free up old head
		return(temp);
	}
}
void pointStackPush(pointStackElement** head, point pt)
{
	if(*head != 0)
	{
		pointStackElement * ele = (pointStackElement *) malloc(sizeof(pointStackElement));		//  Create new head
		ele->elementData = pt;
		ele->nextElement = *head;
		*head = ele;																			//  Update head
	}
	return;
}