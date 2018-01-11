/* =============================================================================
 * 
 * Title:         GPX to GeoJSON converter
 * Author:        Felix Niederwanger
 * License:       Copyright (c), 2015 Felix Niederwanger
 *                MIT license (http://opensource.org/licenses/MIT)
 * Description:   Simple CLI utility to convert GPX to GeoJSON
 * 
 * =============================================================================
 */
 
 
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

#include <stdio.h>
#include "rapidxml.hpp"

using namespace std;
using namespace rapidxml;

/** Coordinate */
class Coords {
private:

public:
    float lon = 0.0F;
    float lat = 0.0F;
    string time;
    string name;
    string cmt;
    string desc;
    string links;
    
    Coords() {}
    Coords(const Coords &src) {
        this->lon = src.lon;
        this->lat = src.lat;
        this->time = src.time;
        this->name = src.name;
        this->cmt = src.cmt;
        this->desc = src.desc;
        this->links = src.links;
    }
};

static void printGeoJson(ostream &out, const vector<Coords> &coords) {
    out << "{" << endl <<
        "    \"type\": \"FeatureCollection\"," << endl <<
        "    \"features\": [" << endl ;
        
    bool first = true;
    for(vector<Coords>::const_iterator it = coords.begin(); it != coords.end(); it++) {
        if(first) first = false;
        else out << ", ";

        out <<  "        {" << endl <<
                "            \"type\": \"Feature\"," << endl <<
                "            \"properties\": " << endl <<
                "            {" << endl <<
                "                \"name\": \"" << (*it).name << "\"," << endl <<
                "                \"time\": \"" << (*it).time << "\"," << endl <<
                "                \"cmt\": \"" << (*it).cmt << "\"," << endl <<
                "                \"desc\": \"" << (*it).desc << "\"," << endl <<
                "                \"links\": ["<< endl;
        out <<  "                     {" << endl;
        out <<  "                        \"href\": \"" << (*it).name << ".jpg\"" << endl;
        out <<  "                     }" << endl;
        out <<  "                ]" << endl;
        out <<  "            }," << endl  ;
        out << "             \"geometry\": " << endl;
        out << "             {" << endl;
        out << "                \"type\": \"Point\"," << endl;
        out << "                \"coordinates\": [" << endl;
        out << "                        " << (*it).lon << "," << endl;
        out << "                        " << (*it).lat << endl;
        out << "                    ]" << endl;
        out << "             }" << endl;
        out << "         }" << endl;
    }
    

    out << endl;
    out << "     ]," << endl;
    out << "     \"threshold\" : 100" << endl;
    out << "}" << endl;
}

static bool processStream(istream &in, ostream &out = cout) {
    // Read stream to end
    stringstream buf;
    string text;
    while((getline(in, text)))
        buf << text << '\n';
    text = buf.str();
    
    
    // Parse GPX
    try {
        xml_document<> doc;                     // character type defaults to char
        doc.parse<0>((char*)text.c_str());      // 0 means default parse flags
        
        xml_node<> *node = doc.first_node("gpx");
        if(node == NULL) throw "No data";
        
        // Get GPX attributes
        map<string, string> attrs;              // GPX attributes
        for (xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
            attrs[attr->name()] = attr->value();

        vector<Coords> coords;

        // Read out wpt
        for (xml_node<> *pt = node->first_node("wpt"); pt; pt = pt->next_sibling("wpt"))
        {


                    Coords coord;

                    for (xml_attribute<> *attr = pt->first_attribute(); attr; attr = attr->next_attribute()) {
                        string name = attr->name();
                        if(name == "lon")
                            coord.lon = ::atof(attr->value());
                        else if(name == "lat")
                            coord.lat = ::atof(attr->value());
                    }
                    for (xml_node<> *p = pt->first_node(); p; p = p->next_sibling()) {
                        string name = p->name();
                        if (name == "time")
                            coord.time = p->value();
                        else if (name == "name")
                            coord.name = p->value();
                        else if (name == "cmt")
                            coord.cmt = p->value();
                        else if (name == "desc")
                            coord.desc = p->value();
                        else if (name == "link")
                            //coord.desc = p->value();
                            for (xml_attribute<> *attr = p->first_attribute(); attr; attr = attr->next_attribute()) {
                                string name = attr->name();
                                if(name == "href")
                                    coord.links = attr->value();
                            }
                    }

                    coords.push_back(coord);


        }
        
        printGeoJson(out, coords);
        
    } catch (parse_error &e) {
        cerr << "Parse error: " << e.what() << endl;
        return false;
    } catch (const char* msg) {
        cerr << "Parse error: No GPX data" << endl;
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    if(argc < 2) {
        cerr << "Reading from stdin until eof" << endl;
        processStream(cin, cout);
    } else {
        string o_filename;      // Output filename, if empty stdout
        
        for(int i=1;i<argc;i++) {
            string arg = argv[i];
            if(arg == "") continue;
            if(arg.at(0) == '-') {
                if(arg == "-h" || arg == "--help") {
                    cout << "GPX to GeoJson converter || 2017, phoenix" << endl << endl;
                    
                    cout << "Usage: " << argv[0] << " [OPTIONS] [FILES]" << endl;
                    cout << "OPTIONS:" << endl;
                    cout << "  -h   --help         Print this help message" << endl;
                    cout << "       --version      Print program version" << endl;
                    cout << "  -o FILE             Write output to FILE" << endl;
                    cout << "                      -o must be before a FILE and matches for all successive files" << endl;
                    cout << "                      It's possible to write different input files to different output files by chaining the commands:" << endl;
                    cout << "                      " << argv[0] << " -o FILE1.json FILE1.gpx -o FILE2.json FILE2.gpx ..." << endl;
                    return EXIT_SUCCESS;
                } else if(arg == "--version") {
                    cout << "1.0" << endl;
                    return EXIT_SUCCESS;
                } else if(arg == "-o") {
                    if(i >= argc-1) {
                        cerr << "Missing argument: Output file" << endl;
                        return EXIT_FAILURE;
                    }
                    o_filename = argv[++i];
                } else {
                    cerr << "Illegal argument: " << arg << endl;
                    return EXIT_FAILURE;
                }
            } else {
                ifstream f_in(argv[i]);
                if(f_in.is_open()) {
                    
                    if (o_filename.size() > 0) {
                        ofstream f_out(o_filename.c_str(), ofstream::out | ofstream::app);
                        if (!f_out.is_open()) {
                            cerr << "Error opening output file " << o_filename << endl;
                            return EXIT_FAILURE;
                        }
                        processStream(f_in, f_out);
                        f_out.close();

                    } else {
                         processStream(f_in, cout);
                    }

                    f_in.close();
                    
                } else {
                    cerr << "Error opening " << argv[i] << endl;
                    return EXIT_FAILURE;
                }   
            }
        }

    }
    return EXIT_SUCCESS;
}
