#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stb_image.h>
#include <stb_image_write.h>
#include "json.hpp"

void GenPBR(std::vector<std::string> &path, std::vector<std::string> &opath)
{
    int w, h, nrChannels;
    unsigned char *mra;
    unsigned char *nh;
    for (int i = 0; i < 3; i++)
    {
        unsigned char *data = stbi_load(path[i].c_str(), &w, &h, &nrChannels, 0);
        if (!data)
        {
            std::cout << path[i] << " ERR" << std::endl;
        }
        if (i == 0)
        {
            //std::cout << "W " << w << " H " << h << " channel " << nrChannels << std::endl;
            mra = new unsigned char[w * h * 3];
            nh = new unsigned char[w * h * 4];
        }
        std::cout << "W " << w << " H " << h << " channel " << nrChannels << std::endl;
        for (int j = 0; j < w; j++)
            for (int k = 0; k < h; k++)
                mra[(j * h + k) * 3 + i] = data[(j * h + k) * nrChannels];
        stbi_image_free(data);
    }

    unsigned char *data = stbi_load(path[3].c_str(), &w, &h, &nrChannels, 0);
    if (!data)
    {
        std::cout << path[3] << " ERR" << std::endl;
    }
    std::cout << "W " << w << " H " << h << " channel " << nrChannels << std::endl;
    for (int j = 0; j < w; j++)
        for (int k = 0; k < h; k++)
            for (int i = 0; i < 3; i++)
                nh[(j * h + k) * 4 + i] = data[(j * h + k) * nrChannels + i];
    stbi_image_free(data);

    data = stbi_load(path[4].c_str(), &w, &h, &nrChannels, 0);
    if (!data)
    {
        std::cout << path[4] << " ERR" << std::endl;
    }
    std::cout << "W " << w << " H " << h << " channel " << nrChannels << std::endl;
    for (int j = 0; j < w; j++)
        for (int k = 0; k < h; k++)
            nh[(j * h + k) * 4 + 3] = data[(j * h + k) * nrChannels];
    stbi_image_free(data);

    stbi_write_png(opath[0].c_str(), w, h, 3, mra, 0);
    stbi_write_png(opath[1].c_str(), w, h, 4, nh, 0);
}

int main(int argc, char *argv[])
{
    std::string path(argv[1]);
    std::cout << path;
    std::ifstream infile(path);
    std::stringstream buffer;
    buffer << infile.rdbuf();
    std::string s(buffer.str());
    infile.close();
    auto j = nlohmann::json::parse(s);
    std::vector<std::string> in(5);
    std::vector<std::string> out(2);
    //in[0] = j["albedo"].get<std::string>();
    in[0] = j["metallic"].get<std::string>();
    in[1] = j["roughness"].get<std::string>();
    in[2] = j["ao"].get<std::string>();
    in[3] = j["normal"].get<std::string>();
    in[4] = j["height"].get<std::string>();
    std::cout << in[4] << std::endl;
    out[0] = j["mra"].get<std::string>();
    out[1] = j["nh"].get<std::string>();
    GenPBR(in, out);
    return 0;
}