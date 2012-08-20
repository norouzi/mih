#include <stdio.h>
#include <math.h>
#include <iostream>
#include "bucket_group.h"

using namespace std;

int main() {
    BucketGroup bg;

    bg.insert(3,303);
    cout << bg.empty << endl;
    bg.group->print();

    bg.insert(3,304);
    cout << bg.empty << endl;
    bg.group->print();
    
    bg.insert(0,100);
    cout << bg.empty << endl;
    bg.group->print();

    bg.insert(0,101);
    cout << bg.empty << endl;
    bg.group->print();

    bg.insert(0,102);
    cout << bg.empty << endl;
    bg.group->print();

    bg.insert(31,3101);
    cout << bg.empty << endl;
    bg.group->print();

    bg.insert(19,1901);
    cout << bg.empty << endl;
    bg.group->print();

    bg.insert(31,3102);
    cout << bg.empty << endl;
    bg.group->print();

    bg.insert(19,1902);
    cout << bg.empty << endl;
    bg.group->print();

    bg.insert(31,3103);
    cout << bg.empty << endl;
    bg.group->print();

    bg.insert(19,1903);
    cout << bg.empty << endl;
    bg.group->print();

    bg.insert(19,1904);
    cout << bg.empty << endl;
    bg.group->print();

    bg.insert(19,1905);
    cout << bg.empty << endl;
    bg.group->print();

    int size=0;
    UINT32 *ret;

    for (int j=0; j<32; j++) {
	ret = bg.query(j, &size);
	printf("%d: ", j);
	for (int i=0; i<size; i++)
	    printf("%d ", ret[i]);
	printf("\n");
    }
}
