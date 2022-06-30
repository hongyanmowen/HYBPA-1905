// HYBPA1905.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//【问题描述】平台中添加cfmesh网格划分工具时需要修改现有文件格式，希望构件一个小工具，快速修改文件格式便于验证；
//
//1.在现有生成的算例文件夹中constant文件夹中，将所有stl文件中第一行“solid ascii”修改为“solid ” + “文件名”（不带stl文件）；
//
//2.将修改后所有的stl文件合并为一个stl文件，名称固定为“jianzu.stl”, 放在主算例目录中;
//
//3.修改system文件夹下的meshDict文件，替换和新建其中renameBoundary文件中的文件名称（constan文件夹下的所有stl文件名，这里不带.stl）;
//
//4.修改system文件夹下的meshDict文件，替换和新建其中localRefinement文件中的文件名称（constan文件夹下的所有stl文件名，这里不带.stl）格式一致;

#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include<iostream>
#include<string>
#include<vector>
#include<direct.h>
#include <io.h>
#include <fstream>
using namespace std;
// 获取exe所在路径
string GetExePath()
{
    char szFilePath[MAX_PATH + 1] = { 0 };
    GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
    /*
    strrchr:函数功能：查找一个字符c在另一个字符串str中末次出现的位置（也就是从str的右侧开始查找字符c首次出现的位置），
    并返回这个位置的地址。如果未能找到指定字符，那么函数将返回NULL。
    使用这个地址返回从最后一个字符c到str末尾的字符串。
    */
    (strrchr(szFilePath, '\\'))[0] = 0; // 删除文件名，只获得路径字串//
    string path = szFilePath;
    return path;
}

//获取Constant Stl文件名，不带 .stl
void getJustConstantStlFile(string path, vector<string>& files)
{
    path += "\\constant\\triSurface";
    //文件句柄
    long hFile = 0;
    //文件信息
    struct _finddata_t fileinfo;//文件信息读取结构
    string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do {
            if ((fileinfo.attrib & _A_SUBDIR)) {//文件类型是不是目录
                ;
            }
            else
            {
                //(strrchr(fileinfo.name, '\\'))[0] = 0; // 删除文件名，只获得路径字串//
                string strName = fileinfo.name;
                if (-1 != strName.find(".stl"))
                {
                    strName.erase(strName.end() - 4, strName.end());
                    files.push_back(strName);
                }
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}

// 读stl文件合并成 jianzhu.stl
bool ReadAndMergeStlFile(string path, vector<string> stlNameList)
{
    char cWorkPath[MAX_PATH];
    char cLineBuffer[MAX_PATH];
    string sAppConfig;				// AppConfig.txt 的路径
    string sLineBuffer;				// 每一行的字符串
    string sWriteBuffer;
    BOOL bFindKey = FALSE;			// 找到关键词为 TRUE，否则为 FALSE

    ifstream ifsReader;    // 读取原始文件
    ofstream ofRewrite;    // 输出修改后文件
    ofstream ofWriteJianzhu;    // 合并建筑文件

    string strJianzhuPath = path + "\\jianzhu.stl";

    string strStlPath = path + "\\constant\\triSurface\\";

    ofWriteJianzhu.open(strJianzhuPath, ios::out | ios::binary);
    ofWriteJianzhu.flush();

    // 便利所有stl文件，替换首行
    for (auto val : stlNameList)
    {
        string strStl = strStlPath + val + ".stl";
        std::cout << strStl << "\n";

        ifsReader.open(strStl, ios::in | ios::binary);			// 打开 AppConfig.txt
        if (ifsReader.is_open())
        {
            int nFirst = 1;
            while (ifsReader.getline(cLineBuffer, MAX_PATH))
            {
                sLineBuffer = cLineBuffer;
                if (1 == nFirst)								                // 只修改第一行文件名
                {
                    sWriteBuffer += "solid " + val + "\n";	// 将要修改的行替换
                    bFindKey = TRUE;
                }
                else
                {
                    sWriteBuffer += sLineBuffer + "\n";
                }
                nFirst++;
            }
            ifsReader.close();
        }
        else
        {
            // 没有打开文件
            return FALSE;
        }
        ofRewrite.open(strStl.c_str(), ios::out | ios::binary);
        ofRewrite.flush();
        ofRewrite << sWriteBuffer;
        ofRewrite.close();

        ofWriteJianzhu << sWriteBuffer;
        sWriteBuffer.clear();
    }
    ofWriteJianzhu.close();

    return TRUE;
}

// 替换MeshDict下的文件名称
bool ReplaceMeshDict(string path, vector<string> stlNameList)
{
    char cWorkPath[MAX_PATH];
    char cLineBuffer[MAX_PATH];
    string sLineBuffer;				// 每一行的字符串
    string sWriteBuffer;

    ifstream ifsReader;    // 读取原始文件
    ofstream ofRewrite;    // 输出修改后文件
    ofstream ofWriteJianzhu;    // 合并建筑文件
    string strMeshDict = path + "\\system\\meshDict";

    ifsReader.open(strMeshDict, ios::in | ios::binary);			// 打开 AppConfig.txt
    if (ifsReader.is_open())
    {
        int nFirst = 1;
        bool bWriter = true;
        while (ifsReader.getline(cLineBuffer, MAX_PATH))
        {
            sLineBuffer = cLineBuffer;
            if (-1 != sLineBuffer.find("renameBoundary"))								                // 只修改第一行文件名
            {

                sWriteBuffer += sLineBuffer + "\n";
                sWriteBuffer += "{\n";
                sWriteBuffer += " defaulType wall;\n";
                sWriteBuffer += " newPatchNames\n";
                sWriteBuffer += " {\n";

                // 便利所有stl文件，添加新名称
                for (auto val : stlNameList)
                {
                    sWriteBuffer += "    " + val + "\n";
                    sWriteBuffer += "    {\n";
                    sWriteBuffer += "    newName " + val + "\n";
                    sWriteBuffer += "    type patch;\n";
                    sWriteBuffer += "    }\n";
                }
                sWriteBuffer += " }\n";
                sWriteBuffer += "}\n";

                // 写localRefinement
                sWriteBuffer += "\n";
                sWriteBuffer += "localRefinement\n";
                sWriteBuffer += "{\n";
                // 便利所有stl文件，添加新名称
                for (auto val : stlNameList)
                {
                    sWriteBuffer += "    " + val + "\n";
                    sWriteBuffer += "    {\n";
                    sWriteBuffer += "     cellSize 0.01;\n";
                    sWriteBuffer += "    }\n";
                }
                sWriteBuffer += "}\n";
                sWriteBuffer += "\n";
                sWriteBuffer += "// ************************************************************************* //\n";
                break;
            }
            else
            {
                sWriteBuffer += sLineBuffer + "\n";
            }


        }
        ifsReader.close();
    }
    else
    {
        // 没有打开文件
        return FALSE;
    }
    ofRewrite.open(strMeshDict, ios::out | ios::binary);
    ofRewrite.flush();

    ofRewrite << sWriteBuffer;
    ofRewrite.close();
    sWriteBuffer.clear();

    return true;
}

int main()
{
    string running_path = GetExePath();
    vector<string> stlFilesNameList;

    //获取Constant Stl文件名，不带 .stl
    getJustConstantStlFile(running_path, stlFilesNameList);

    // 读stl文件合并成 jianzhu.stl
    ReadAndMergeStlFile(running_path, stlFilesNameList);

    // 替换MeshDict下的文件名称
    ReplaceMeshDict(running_path, stlFilesNameList);
    std::cout << "Hello World!\n";

    return 0;
}

