#ifndef LICENSE_H
#define LICENSE_H

#include <string>

#define LICENSE_NAME_LENGTH 20
#define LICENSE_TYPE_LENGTH 4
#define LICENSE_COPYRIGHT_LENGTH 30

struct license {
    char name[LICENSE_NAME_LENGTH];
    char type[LICENSE_TYPE_LENGTH];
    char copyright[LICENSE_COPYRIGHT_LENGTH];
    char url[100];
    std::string original;
};

const license Licenses[] = {
        {
            "sfinder-cpp",
            "MIT",
            "Copyright (c) 2019 knewjade",
            "https://github.com/knewjade/sfinder-cpp/",
            "MIT License\n"
            "\n"
            "Copyright (c) 2019 knewjade\n"
            "\n"
            "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
            "of this software and associated documentation files (the \"Software\"), to deal\n"
            "in the Software without restriction, including without limitation the rights\n"
            "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
            "copies of the Software, and to permit persons to whom the Software is\n"
            "furnished to do so, subject to the following conditions:\n"
            "\n"
            "The above copyright notice and this permission notice shall be included in all\n"
            "copies or substantial portions of the Software.\n"
            "\n"
            "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
            "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
            "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
            "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
            "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
            "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
            "SOFTWARE."
        }
};

#endif //LICENSE_H
