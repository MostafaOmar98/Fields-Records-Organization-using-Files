/*
 * Author: Mostafa Omar Mahmoud El-Sayed
 * ID: 20170292
 * G: 11
 * College assignment to organize Fields and Records using files in C++.
    Records represent Books with the following Fields: ISBN, Title, Author Name, Price, Year Published, Number of Pages.
    Method of Organization: Fixed length records, Delimeted records.

    Operations:
    1- Add Record
    2- Print all Records
    3- Print Record (Search by Book Title)
    4- Update record (Search by Book ISBN)
    5- Delete record (Search by Book ISBN) (Doesn't actually delete the actual bytes from the file but marks the record as deleted)
    6- Compact File (Removes deleted record from the file)
    7- Get file size
 */


#include <iostream>
#include <fstream>
#include <cstring>


using namespace std;

const int MAX_STRING_SIZE = 30;
const char delimeter = '|';
const int recordSize = 78, dSize = 6;
struct Book{
    char ISBN[6], title[MAX_STRING_SIZE], authorName[MAX_STRING_SIZE];
    float price;
    int year, numOfPages;
};

/*
 * Reads a line of input, handles if the delimeter was entered right before reading this line
 */
void readLine(istream &in, char s[], int size, char d)
{
    in.getline(s, size, d);
    if (s[0] == '\0')
        in.getline(s, size,d);
}

istream& operator>>(istream &in, Book &b)
{
    cout << "Enter ISBN: ";
    readLine(in, b.ISBN, 6, '\n');
    cout << "Enter title: ";
    readLine(in, b.title, MAX_STRING_SIZE, '\n');
    cout << "Enter Author Name: ";
    readLine(in, b.authorName, MAX_STRING_SIZE, '\n');
    cout << "Enter Price: ";
    in >> b.price;
    cout << "Enter Year: ";
    in >> b.year;
    cout << "Enter Number of Pages: ";
    in >> b.numOfPages;
    return in;
}

ostream &operator<<(ostream &out, const Book &b)
{
    out << "ISBN: ";
    out << b.ISBN << '\n';
    out << "title: ";
    out << b.title << '\n';
    out << "Author Name: ";
    out << b.authorName << '\n';
    out << "Price: ";
    out << b.price << '\n';
    out << "Year: ";
    out << b.year << '\n';
    out << "Number of Pages: ";
    out << b.numOfPages << '\n';
    return out;

}

/*
 * Adds one book to fstream, put pointer is determined before this function is called
 * Fixed length record, delimeted fields
 */
void addBook(fstream &file, const Book &b)
{
    file.write(b.ISBN, sizeof(b.ISBN));
    file.put(delimeter);
    file.write(b.title, sizeof(b.title));
    file.put(delimeter);
    file.write(b.authorName, sizeof(b.authorName));
    file.put(delimeter);
    file.write((char*)&b.price, sizeof(b.price));
    file.put(delimeter);
    file.write((char*)&b.year, sizeof(b.year));
    file.put(delimeter);
    file.write((char*)&b.numOfPages, sizeof(b.numOfPages));
    file.put(delimeter);

}

/*
 * 1 if read correctly
 * 0 if eof
 * -1 if deleted
 */
int readBook(fstream &file, Book &b)
{
    file.getline(b.ISBN, 7, delimeter);
    if (file.eof() || file.fail())
    {
        file.clear();
        return false;
    }
    file.getline(b.title, MAX_STRING_SIZE + 1, delimeter);
    file.getline(b.authorName, MAX_STRING_SIZE + 1, delimeter);
    file.read((char*)&b.price, sizeof(b.price));
    file.ignore();
    file.read((char*)&b.year, sizeof(b.year));
    file.ignore();
    file.read((char*)&b.numOfPages, sizeof(b.numOfPages));
    file.ignore();
    if (b.ISBN[0] == '*')
        return -1;
    return true;
}


/*
 * Reads 2 header byes to get first freeRRN space
 * If no header exists, adds -1 as a header
 */
short getFreeRRN(const char fileName[])
{
    fstream file(fileName, ios::out|ios::in|ios::binary);
    short freeRRN = -1;
    file.read((char*)&freeRRN, sizeof(freeRRN));
    if (file.eof()) // if file is empty and doesn't contain initial header, add header
    {
        file.clear();
        file.seekp(0, ios::beg);
        freeRRN = -1;
        file.write((char*)&freeRRN, sizeof(freeRRN));
    }
    file.close();
    return freeRRN;
}

void addNBooks(const char fileName[], int n)
{
    short freeRRN = getFreeRRN(fileName);
    fstream file(fileName, ios::out|ios::in|ios::binary);
    file.seekg(2, ios::beg);
    Book b;
    while(n--)
    {
        cin >> b;
        if (freeRRN != -1)
        {
            file.seekp(2 + freeRRN * (recordSize + dSize) + 1, ios::beg); // To the free record, +2 because of header, +1 to skip the '*'
            file.read((char*)&freeRRN, sizeof(freeRRN));
            file.seekp(-3, ios::cur); // -2 for the next RRN, -1 for the '*'
            addBook(file, b);
            file.seekp(0, ios::beg);
            file.write((char*)&freeRRN, sizeof(freeRRN));
        }
        else
        {
            file.seekp(0, ios::end);
            addBook(file, b);
        }

    }
    file.close();
}


void printAllBooks(const char fileName[])
{
    fstream file(fileName, ios::in|ios::out|ios::binary);
    getFreeRRN(fileName);
    file.seekg(2, ios::beg);
    Book b;
    while(readBook(file, b))
    {
        if (b.ISBN[0] == '*')
            continue;
        cout << b << '\n';
    }
    file.clear();
    file.close();
}

bool printBook(const char fileName[], const char title[])
{
    fstream file(fileName, ios::out|ios::in|ios::binary);
    getFreeRRN(fileName);
    file.seekg(2, ios::beg);
    Book b;
    bool found = 0;
    while(readBook(file, b))
    {
        if (b.ISBN[0] == '*')
            continue;
        if (strcmp(b.title, title) == 0)
        {
            found = 1;
            cout << b << '\n';
            break;
        }
    }
    file.clear();
    file.close();
    return found;
}

bool deleteBook(const char fileName[], const char ISBN[])
{
    fstream file(fileName, ios::in|ios::out|ios::binary);
    short freeRRN = getFreeRRN(fileName);
    file.seekg(2, ios::beg);
    Book b;
    bool found = 0;
    while(readBook(file, b))
    {
        if (b.ISBN[0] == '*')
            continue;
        if (strcmp(b.ISBN, ISBN) == 0)
        {
            found = 1;
            file.seekg(-(recordSize + dSize), ios::cur);
            file.put('*');
            file.write((char*)&freeRRN, sizeof(freeRRN));
            freeRRN = file.tellg()/(recordSize + dSize);
            file.seekp(0, ios::beg);
            file.write((char*)&freeRRN, sizeof(freeRRN));
            break;
        }
    }
    file.close();
    file.clear();
    return found;
}

bool updateBook(const char fileName[], const char ISBN[], Book &newBook)
{
    fstream file(fileName, ios::in|ios::out|ios::binary);
    getFreeRRN(fileName);
    file.seekg(2, ios::beg);
    Book b;
    bool found = 0;
    while(readBook(file, b))
    {
        if (b.ISBN[0] == '*')
            continue;
        if (strcmp(b.ISBN, ISBN) == 0)
        {
            found = 1;
            file.seekg(-(recordSize + dSize), ios::cur);
            addBook(file, newBook);
            break;
        }
    }
    file.clear();
    file.close();
    return found;
}


int getFileSize(const char fileName[])
{
    fstream f(fileName, ios::binary|ios::out|ios::in);
    f.seekg(0, ios::end);
    int ret = f.tellg();
    f.close();
    return ret;
}

/*
 * copies the contest of fileName2 to fileName1
 */
void fileCopy(const char fileName1[], const char fileName2[])
{
    ofstream file1(fileName1, ios::binary);
    ifstream file2(fileName2, ios::binary);

    int size = getFileSize(fileName2);
    char* buffer = new char[size];
    file2.read(buffer, size);
    file1.write(buffer, size);
    delete[] buffer;
}


void compactFile(const char fileName[])
{
    fstream file1(fileName, ios::out|ios::in|ios::binary);
    fstream file2("temporary.txt", ios::trunc|ios::out|ios::binary);
    getFreeRRN("temporary.txt");
    file1.seekg(2, ios::beg);
    file2.seekp(2, ios::beg);
    Book b;
    while(readBook(file1, b))
    {
        if (b.ISBN[0] != '*')
            addBook(file2, b);
    }
    file1.close();
    file2.close();
    fileCopy(fileName, "temporary.txt");
}


void printMenu()
{
    cout << "\nWhat would you like to do now?\n"
            "1-Open new file\n"
            "2-Add N Books\n"
            "3-Print all Books\n"
            "4-Print Book by Title\n"
            "5-Update Book\n"
            "6-Delete Book\n"
            "7-Compact File\n"
            "8-Get file size\n"
            "0-Exit\n"
            "\nChoice: ";
}


int main()
{
    int choice;
    do{
        printMenu();
        cin >> choice;
        cin.ignore();
        char fileName[MAX_STRING_SIZE];
        switch(choice)
        {
            case 1:
            {
                cout << "Enter File Name (Must be created on drive): ";
                cin >> fileName;
                strcat(fileName, ".txt");
                cout << "File Opened!\n";
                break;
            }
            case 2:
            {
                int nBooks;
                cout << "Enter number of Books to enter: ";
                cin >> nBooks;
                cin.ignore();
                addNBooks(fileName, nBooks);
                cout << "Books added!\n";
                break;
            }
            case 3:
            {
                printAllBooks(fileName);
                break;
            }
            case 4:
            {
                char title[MAX_STRING_SIZE];
                cout <<"Enter book title: ";
                readLine(cin, title, MAX_STRING_SIZE, '\n');
                if (!printBook(fileName, title))
                    cout << "Book not found!\n";
                break;
            }
            case 5:
            {
                cout << "Enter Book ISBN: ";
                char ISBN[6];
                readLine(cin, ISBN, 6, '\n');
                cout << "Enter New Book\n";
                Book b;
                cin >> b;
                if (updateBook(fileName,ISBN, b))
                    cout << "Book Updated!\n";
                else
                    cout << "Book not found!\n";
                break;
            }
            case 6:
            {
                cout << "Enter Book ISBN: ";
                char ISBN[6];
                readLine(cin, ISBN, 6, '\n');
                if (deleteBook(fileName, ISBN))
                    cout << "Book deleted!\n";
                else
                    cout << "Book not found!\n";
                break;
            }
            case 7:
            {
                compactFile(fileName);
                cout << "File compacted!\n"
                        "File size is now: " << getFileSize(fileName) << " bytes\n";
                break;
            }
            case 8:
            {
                cout << "File size is: " << getFileSize(fileName) << " bytes\n";
                break;
            }
            case 0:
            {
                cout << "Goodbye!\n";
                break;
            }
            default:
            {
                cout << "Invalid choice\n";
                break;
            }
        }
    }while(choice);
    return 0;
}