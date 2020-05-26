// This code reads input data file by fragments of size 128M (c_fragmentSize)
// sort them separately and merge files to the single one
//
// Data: 25.12.2019
// Author: Petrov V.

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

const char* c_inputFileName = "input";
const char* c_outputFileName = "output";

const unsigned c_fragmentSize = 4;

bool anythingIsRead(const vector<pair<bool, int>>& lastInts) {
    bool read = false;
    for (unsigned i = 0; i < lastInts.size(); ++i)
        read |= lastInts[i].first;
    return read;
}

int compare(const void* x, const void* y){
    return *(int*)x > *(int*)y;
}

int main(int argc, char* argv[]){    
    FILE* infile = fopen("input", "r"); // TODO: make check

    void* buffer = malloc(c_fragmentSize); // TODO: make check

    unsigned no = 0;
    // read fragments one by one and write them to separate files
    while (size_t fragment = fread(buffer, sizeof(int), c_fragmentSize / sizeof(int), infile)) {
        ++no;
        // sort fragment read
        qsort(buffer, fragment, sizeof(int), compare);
        ostringstream sstream;
        sstream << "output_" << no; // write fragment to corresponding file
        const string filename = sstream.str();
        FILE* outfile = fopen(filename.c_str(), "w+"); // TODO: make check
        fwrite(buffer, fragment, sizeof(int), outfile); // TODO: make check
        fclose(outfile);
    }
    fclose(infile);

    // now 'no' is the number of fragments written
    free(buffer);

    if (no) { // if at least one fragment if read
        // open all files with fragments for reading
        vector<FILE*> fragments;
        for (unsigned i = 1; i <= no; ++i) {
            ostringstream sstream;
            sstream << "output_" << i;
            const string filename = sstream.str();
            fragments.push_back(fopen(filename.c_str(), "r")); // TODO: make check
        }

        vector<pair<bool, int>> lastInts(fragments.size());
        FILE* outfile = fopen("output", "w+"); // TODO: make check
        for (unsigned i = 0; i < no; ++i)
                lastInts[i].first = !!fread(&lastInts[i].second, sizeof(int), 1, fragments[i]); // TODO: make check
        while (true) {
            if (!anythingIsRead(lastInts)) // check if anything is read
                break;

            // search one existed yet read value
            int minNo = 0;
            int minRead;
            for (unsigned i = 0; i < no; ++i)
                if (lastInts[i].first)
                    minRead = lastInts[i].second;

            // search for min value
            for (unsigned i = 0; i < no; ++i)
                if (lastInts[i].first && (lastInts[i].second <= minRead)) {
                    minNo = i;
                    minRead = lastInts[i].second;
                }
            // read the next fragment instead of used one
            lastInts[minNo].first &= !!fread(&lastInts[minNo].second, sizeof(int), 1, fragments[minNo]); // TODO: make check

            // write to the output current minimum value 
            fwrite(&minRead, sizeof(int), 1, outfile);
        }
        fclose(outfile);
    
        // close all files with fragment
        for (unsigned i = 0; i < no; ++i)
            fclose(fragments[i]);
    }
}
