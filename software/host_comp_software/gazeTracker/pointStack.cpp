#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include "pointStack.h"

point pointStackPop(pointStackElement** head)
{
	if(*head != 0)
	{
		pointStackElement* tempLink = *head;		//  Remember old head
		point tempData = tempLink->elementData;		//  Grab data
		*head = tempLink->nextElement;				//  Update head
		free(tempLink);								//  Free up old head
		return(tempData);
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

void pointStackPrint(pointStackElement* head)
{
	if(head == 0)
	{
		printf("Stack Empty\n");
		return;
	}
	while(head != 0)
	{
		printf("X:%u Y:%u\n",head->elementData.x, head->elementData.y);
		head = head->nextElement;
	}
}