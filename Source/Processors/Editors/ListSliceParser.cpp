/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2013 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "ListSliceParser.h"
#include <vector>


ListSliceParser::ListSliceParser(std::string s)
{
    defaultString = s;
}

ListSliceParser::~ListSliceParser()
{

}

int ListSliceParser::convertToInteger(std::string s)
{
    char ar[20];
    int i, j, k = 0;
    for (i = 0; i < s.size(); i++)
    {
        if (s[i] >= 48 && s[i] <= 57)
        {
            ar[k] = s[i];
            k++;
        }
    }
    if (k>7)
    {
        return 1000000;
    }
    ar[k] = '\0';
    k = atoi(ar);
    return k;
}

std::vector<int> ListSliceParser::parseStringIntoRange(std::string textBoxInfo,int rangeValue)
{
    std::string s = ",";
    s += textBoxInfo;
    std::vector<int> finalList, separator, rangeseparator;
    int i, j, a, b, k, openb, closeb, otherchar, x, y;
    s += ",";
    for (i = 0; i < s.size(); i++)      //split string by ' , ' or ' ; '
    {
        if (s[i] == ';' || s[i] == ',')
        {
            separator.push_back(i);
        }
    }
    for (i = 0; i < separator.size() - 1; i++)  // split ranges by ' : ' or ' - '
    {
        j = k = separator[i] + 1;
        openb = closeb = otherchar = 0;
        rangeseparator.clear();
        for (; j < separator[i + 1]; j++)
        {
            if (s[j] == '-' || s[j] == ':')
            {
                rangeseparator.push_back(j);
            }
            else if (((int)s[j] == 32))
            {
                continue;
            }
            else if (s[j] == '[' || s[j] == '{' || s[j] == '(')
            {
                openb++;
            }
            else if (s[j] == ']' || s[j] == '}' || s[j] == ')')
            {
                closeb++;
            }
            else if ((int)s[j] > 57 || (int)s[j] < 48)
            {
                otherchar++;
            }
        }

        if (openb != closeb || openb > 1 || closeb > 1 || otherchar > 0)  //Invalid input
        {
            continue;
        }


        for (x = separator[i] + 1; x < separator[i + 1]; x++)       //trim whitespace and brackets from front
        {
            if (((int)s[x] >= 48 && (int)s[x] <= 57) || s[x] == ':' || s[x] == '-')
            {
                break;
            }
        }
        for (y = separator[i + 1] - 1; y > separator[i]; y--)       //trim whitespace and brackets from end
        {
            if (((int)s[y] >= 48 && (int)s[y] <= 57) || s[y] == ':' || s[y] == '-')
            {
                break;
            }
        }
        if (x > y)
        {
            continue;
        }


        if (rangeseparator.size() == 0)   //syntax of form - x or [x]
        {
            a = convertToInteger(s.substr(x, y - x + 1));
            if (a == 0 || a>rangeValue)
            {
                continue;
            }
            finalList.push_back(a - 1);
            finalList.push_back(a - 1);
            finalList.push_back(1);
        }
        else if (rangeseparator.size() == 1) // syntax of type - x-y or [x-y]
        {
            a = convertToInteger(s.substr(x, rangeseparator[0] - x + 1));
            b = convertToInteger(s.substr(rangeseparator[0], y - rangeseparator[0] + 1));
            if (a == 0)
            {
                a = 1;
            }
            if (b == 0)
            {
                b = rangeValue;
            }
            if (a > b || a > rangeValue || b > rangeValue)
            {
                continue;
            }
            finalList.push_back(a - 1);
            finalList.push_back(b - 1);
            finalList.push_back(1);
        }
        else if (rangeseparator.size() == 2)   // syntax of type [x:y:z] or x-y-z
        {
            a = convertToInteger(s.substr(x, rangeseparator[0] - x + 1));
            k = convertToInteger(s.substr(rangeseparator[0], rangeseparator[1] - rangeseparator[0] + 1));
            b = convertToInteger(s.substr(rangeseparator[1], y - rangeseparator[1] + 1));
            if (a == 0)
            {
                a = 1;
            }
            if (b == 0)
            {
                b = rangeValue;
            }
            if (k == 0)
            {
                k = 1;
            }
            if (a > b || a > rangeValue || b > rangeValue)
            {
                continue;
            }
            finalList.push_back(a - 1);
            finalList.push_back(b - 1);
            finalList.push_back(k);
        }
    }
    return finalList;
}