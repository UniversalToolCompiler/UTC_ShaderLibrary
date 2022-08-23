/** Copyright 2022, Universal Tool Compiler */

#include "UTC_Utils.h"

FString FUTC_Utils::PascalToText(const FString PascalText)
{
	FString Text;
	for (int32 i = 0; i<PascalText.Len(); ++i )
	{
		FString CurrentChar = PascalText.Mid(i, 1);
		
		if(i > 0)
		{
			if(CurrentChar != " " && isupper(PascalText[i]) && islower(PascalText[i-1]))
				Text += " ";
		}
		Text += CurrentChar;
	}
	return Text;
}

FString FUTC_Utils::TextToPascal(const FString Text)
{
	FString PascalText;
	for (int32 i = 0; i<Text.Len(); ++i )
	{
		FString CurrentChar = Text.Mid(i, 1);
		
		if(CurrentChar != " ")
			PascalText += CurrentChar;
	}
	return PascalText;
}
