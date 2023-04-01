#include <iostream>
#include <fstream>
#include <iomanip>

int main() {
    std::ofstream outfile("new_plane.obj");

    outfile << std::setprecision(6) << std::fixed;

    const int dimensionX = 10;
    const int dimensionZ = 10;

    outfile << "# Mesh Id\n";
    outfile << "o plane\n";

    outfile << '\n';

    outfile << "# Vertex positions\n";

    float s = (1.0f / dimensionX);
    float t = (1.0f / dimensionZ);    

    for (int z = 0; z < dimensionZ; z++) {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
        for (int x = 0; x < dimensionX; x++) {
                // 1
                outfile << "v " << -0.5f + s*float(x) << " " 
                        << 0.0f << " " 
                        << -0.5f + t*float(z) + t << "\n";
                // 2
                outfile << "v " << -0.5f + s*float(x) << " " 
                        << 0.0f << " " 
                        << -0.5f + t*float(z) << "\n";
                // 3
                outfile << "v " << -0.5f + s*float(x) + s << " " 
                        << 0.0f << " " 
                        << -0.5f + t*float(z) << "\n";
                // 4
                outfile << "v " << -0.5f + s*float(x) + s << " " 
                        << 0.0f << " " 
                        << -0.5f + t*float(z) + t << "\n";
        }
    }

    outfile << '\n';
    outfile << "# Vertex normals\n";

    outfile << "vn " << "0.0" << " " 
                    << "1.0" << " " 
                    << "0.0" << "\n";
    outfile << '\n';

    outfile << "# Triangle face indices: vertex//normal\n";
    outfile << "# Indices start at 1\n";

//     for (int z = 0; z < dimensionZ; z++) {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
//         for (int x = 0; x < dimensionX; x++) {
//                 outfile << "f " << 1 + 4*(dimensionZ) << "//" << 1 << " " 
//                         << 4 + 4*() << "//" << 1 << " " 
//                         << 3 + + 4*()<< "//" << 1 << "\n";
//         }
//     }

    for (int i = 0; i < dimensionX*dimensionZ; i++) {
                outfile << "f " << 1 + 4*i<< "//" << 1 << " " 
                        << 4 + 4*i << "//" << 1 << " " 
                        << 3 + 4*i << "//" << 1 << "\n";

                outfile << "f " << 1 + 4*i<< "//" << 1 << " " 
                        << 2 + 4*i << "//" << 1 << " " 
                        << 3 + 4*i << "//" << 1 << "\n";
    }

    return 0;
}
