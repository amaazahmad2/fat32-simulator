#include<iostream>
#include<string>
using namespace std;

template<class T>
class node
{
public:
	T data;
	node<T>* next;
	

	node()
	{
		next = nullptr;
	}

	node(T d, node<T>* n)
	{
		data = d;
		next = n;
	}

	node(T d)
	{
		data = d;
		next = nullptr;
	}

	template<class T> friend class list;
	//friend void printListOfBlocks(list<block>);
};

template<class T>
class list
{
	node<T>* head;
	node<T>* tail;
	int size;

public:
	list()
	{
		head = new node<T>;
		tail = new node<T>;
		head->next = tail;
		tail->next = nullptr;
		size = 0;
	}

	list(node<T>* h, node<T>* t, int s)
	{
		head = h;
		tail = t;
		size = s;
	}

	void addItem(node<T> obj)
	{
		if (head->next == nullptr)
		{
			head->next = new node<T>;
			head->next->data = obj.data;
			head->next->next = tail;
			
			size++;
		}
		else
		{
			node<T>* temp = head;
			while (temp->next != tail)
			{
				temp = temp->next;
			}
			temp->next = new node<T>;
			temp->next->data = obj.data;
			temp->next->next = tail;
			size++;
		}
	}

	~list()
	{
		head = tail = nullptr;
		size = 0;
	}

	friend void printListOfBlocks(list);
	friend class file;
	friend class fileSystem;
};

class block
{
	int startingSectorID;
	int totalSectors;

public:
	block(int id, int total)
	{
		startingSectorID = id;
		totalSectors = total;
	}

	block()
	{
		startingSectorID = totalSectors = 0;
	}


	friend void printListOfBlocks(list<block>);
	friend class file;
	friend class fileSystem;
};

class file
{
	char*name;
	double size; //size in bytes
	list<block> blocksInFile;

public:
	file()
	{
		name = nullptr;
		size = 0;

	}

	~file()
	{
		delete [] name;
		node<block>*temp = blocksInFile.head;
		node<block>*tempNext = blocksInFile.head;
		while (temp != nullptr)
		{
			tempNext = temp->next;
			delete temp;
			temp = tempNext;
		}
		delete temp;
		//delete tempNext;
	}

	friend class fileSystem;
};

class fileSystem
{
	list<block> pool;
	list<file> filesInSystem;
	char* disk;
	int numOfSectors, sizeOfSector, numOfSectorsInPool;

	void divideTheBlock(node<block>* &tempBlock, int sectorsRequired)
	{
		node<block>*addressForNextBlock = tempBlock->next;
		tempBlock->next = new node<block>;
		tempBlock->next->data.totalSectors = tempBlock->data.totalSectors - sectorsRequired; //num of sectors for newly created block
		tempBlock->data.totalSectors = tempBlock->data.totalSectors - tempBlock->next->data.totalSectors; //num of sectors for previously existing block (in which data will be saved)
		tempBlock->next->data.startingSectorID = tempBlock->data.startingSectorID + sectorsRequired;
		tempBlock->next->next = addressForNextBlock;
	}

	void mergeBlocksInPool()
	{
		node<block>*currentBlock = pool.head;
		node<block>*nextBlock = pool.head->next;
		numOfSectorsInPool = 0;

		while (nextBlock != pool.tail)
		{
			if (currentBlock->data.startingSectorID + currentBlock->data.totalSectors == nextBlock->data.startingSectorID)
			{
				currentBlock->next = nextBlock->next;
				currentBlock->data.totalSectors = currentBlock->data.totalSectors + nextBlock->data.totalSectors;

				currentBlock = pool.head;
				nextBlock = pool.head->next;
				numOfSectorsInPool = 0;
				continue;
			}
			numOfSectorsInPool = numOfSectorsInPool + nextBlock->data.totalSectors;
			currentBlock = currentBlock->next;
			nextBlock = nextBlock->next;
		}

		
	}

public:
	fileSystem(int num_of_sectors, int size_of_sector)
	{
		numOfSectors = numOfSectorsInPool = num_of_sectors;
		sizeOfSector = size_of_sector;
		disk = new char[numOfSectors*sizeOfSector];
		memset(disk, '\0', numOfSectors*sizeOfSector);
		pool.head->next = new node<block>;
		pool.head->next->data.totalSectors = numOfSectors;
		pool.head->next->data.startingSectorID = 0;
		pool.head->next->next = pool.tail;
		pool.head->data.totalSectors = 0;
		pool.tail->data.totalSectors = 0;
		
		filesInSystem.head->next = filesInSystem.tail;
		filesInSystem.head->data.name = new char[1];
		filesInSystem.tail->data.name = new char[1];
		filesInSystem.head->data.name[0] = '\0';
		filesInSystem.tail->data.name[0] = '\0';

		pool.head->data.startingSectorID = -1;
		pool.head->data.totalSectors = -1;
		pool.tail->data.startingSectorID = -1;
		pool.tail->data.totalSectors = -1;
	}

	bool saveFile(char* fileName, char* fileContent, double fileSize)
	{
		//node<file>* nextAddress = filesInSystem.head->next;

		//filesInSystem.head->next = new node<file>;
		//filesInSystem.head->next->next=nextAddress;

		node<file>* tempFile = filesInSystem.head;  //to traverse through all the files and compare the names
		while (tempFile != filesInSystem.tail)
		{
			if (strcmp(tempFile->data.name,fileName)==0)
			{
				cout << endl << "A FILE WITH THIS NAME ALREADY EXISTS" << endl;
				return false;
			}
			tempFile = tempFile->next;
		}

		tempFile = filesInSystem.head;
		while (tempFile->next != filesInSystem.tail)
		{
			tempFile = tempFile->next;
		}

		if (fileSize > (numOfSectorsInPool*sizeOfSector)) 
			return false;

		int sectorsRequiredToStoreFile = ceil(fileSize / sizeOfSector);

		if (numOfSectorsInPool < sectorsRequiredToStoreFile)
			return false;

		tempFile->next = new node<file>;
		node<file>* newFile = tempFile->next;
		newFile->data.name = new char[strlen(fileName)+1];
		strcpy_s(newFile->data.name, strlen(fileName)+1,fileName);
		newFile->data.size = fileSize;
		newFile->next = filesInSystem.tail;

		node<block>*tempBlock = pool.head->next;
		if (tempBlock->data.totalSectors > sectorsRequiredToStoreFile)
		{
			divideTheBlock(tempBlock, sectorsRequiredToStoreFile);
			node<block>*h = newFile->data.blocksInFile.head;
			while (h->next != newFile->data.blocksInFile.tail)
			{
				h = h->next;
			}
			pool.head->next = tempBlock->next;
			h->next = tempBlock;
			tempBlock->next = newFile->data.blocksInFile.tail;
			numOfSectorsInPool = numOfSectorsInPool - sectorsRequiredToStoreFile;

		}
		//if we dont find the required num of sectors in the first block, we start counting the available num of sectors in every block
		//ahead and stop when've found the required num of sectors, irrespective of number of blocks
		else 
		{
			int availableSectorsFound = 0;
			while (true)
			{
				availableSectorsFound = availableSectorsFound + tempBlock->data.totalSectors;

				if (availableSectorsFound >= sectorsRequiredToStoreFile)
					break;

				tempBlock = tempBlock->next;
			}
			if(availableSectorsFound-sectorsRequiredToStoreFile != 0)
				divideTheBlock(tempBlock, availableSectorsFound - sectorsRequiredToStoreFile);

			node<block>*h = newFile->data.blocksInFile.head;
			while (h->next != newFile->data.blocksInFile.tail)
			{
				h = h->next;
			}
			h->next = pool.head->next;
			pool.head->next = tempBlock->next;
			tempBlock->next = newFile->data.blocksInFile.tail;
			numOfSectorsInPool = numOfSectorsInPool - sectorsRequiredToStoreFile;



			
		}
		tempBlock = newFile->data.blocksInFile.head->next;
		int i = 0;

		int sectorToWriteData = tempBlock->data.startingSectorID;
		int lastSectorInCurrentBlock = tempBlock->data.startingSectorID + tempBlock->data.totalSectors;
		int sectorOffset = 0;
		//h = newFile->data.blocksInFile.head->next;
		while (i < strlen(fileContent))
		{
			disk[(sectorToWriteData*sizeOfSector) + sectorOffset] = fileContent[i];
			i++;

			sectorOffset++;
			if (sectorOffset == sizeOfSector)
			{
				sectorToWriteData++;
				sectorOffset = 0;
			}
			if (sectorToWriteData >= lastSectorInCurrentBlock)
			{
				tempBlock = tempBlock->next;
				if (tempBlock == nullptr)
					break;
				sectorToWriteData = tempBlock->data.startingSectorID;
				lastSectorInCurrentBlock = tempBlock->data.startingSectorID + tempBlock->data.totalSectors;
			}

		}
		
		return true;
		
	}

	void readFile(char*file_name)
	{
		bool fileFound = false;
		node<file>* tempFile = filesInSystem.head->next;  //to traverse through all the files and compare the names
		while (tempFile != filesInSystem.tail)
		{
			if (strcmp(tempFile->data.name, file_name)==0)
			{
				fileFound = true;
				break;
			}
			tempFile = tempFile->next;
		}
		if (fileFound == false)
		{
			cout <<endl<< "FILE NOT FOUND" << endl << endl;
		}
		else
		{
			node<block>*tempBlock = tempFile->data.blocksInFile.head->next;
			int sectorToReadData = tempBlock->data.startingSectorID;
			int lastSectorInCurrentBlock = tempBlock->data.startingSectorID + tempBlock->data.totalSectors;
			int sectorOffset = 0;
			//h = newFile->data.blocksInFile.head->next;
			while (tempBlock->next != nullptr)
			{
				cout << disk[(sectorToReadData*sizeOfSector) + sectorOffset];

				sectorOffset++;
				if (sectorOffset == sizeOfSector)
				{
					sectorToReadData++;
					sectorOffset = 0;
				}
				if (sectorToReadData >= lastSectorInCurrentBlock)
				{
					tempBlock = tempBlock->next;

					if (tempBlock == nullptr)
						break;
					sectorToReadData = tempBlock->data.startingSectorID;
					lastSectorInCurrentBlock = tempBlock->data.startingSectorID + tempBlock->data.totalSectors;
					
				}

			}
		}
	}

	void readAllFiles()
	{
		node<file>* tempFile = filesInSystem.head->next;
		while (tempFile != filesInSystem.tail)
		{
			cout << endl<<endl<<"FILE NAME: " << tempFile->data.name << endl;
			cout << "CONTENTS: ";
			readFile(tempFile->data.name);
			cout << endl << endl;

			tempFile = tempFile->next;
		}
	}

	void deleteFile(char*file_name)
	{
		bool fileFound = false;
		node<file>* tempFile = filesInSystem.head->next; //to traverse through all the files and compare the names
		node<file>* prevFile = filesInSystem.head;
		while (tempFile != filesInSystem.tail)
		{
			if (strcmp(tempFile->data.name, file_name) == 0)
			{
				fileFound = true;
				break;
			}
			tempFile = tempFile->next;
			prevFile = prevFile->next;
		}
		if (fileFound == false)
		{
			cout << endl << "FILE NOT FOUND" << endl << endl;
			
			return;
		}
		else
		{
			prevFile->next = tempFile->next;
			node<block>* blockToBeTransferred = tempFile->data.blocksInFile.head->next;
			while (blockToBeTransferred != tempFile->data.blocksInFile.tail)
			{
				bool previousBlockFound = false;
				int startingSector = blockToBeTransferred->data.startingSectorID;
				int numOfSectors = blockToBeTransferred->data.totalSectors;
				node<block>* nextBlockToBeTranferred = blockToBeTransferred->next;

				node<block>* blockFromPool = pool.head->next;
				node<block>* previousBlockFromPool = pool.head;
				while (blockFromPool != pool.tail)
				{
					if (blockFromPool->data.startingSectorID >= startingSector )
					{
						previousBlockFound = true;
						break;
					}
					previousBlockFromPool = blockFromPool;
					blockFromPool = blockFromPool->next;
					previousBlockFound = false;
				}
				if (previousBlockFound == false && startingSector==0)
				{
					node<block>*tempNext = pool.head->next;
					pool.head->next = blockToBeTransferred;
					blockToBeTransferred->next = tempNext;
				}
				else
				{
					node<block>*tempNext = previousBlockFromPool->next;
					previousBlockFromPool->next = blockToBeTransferred;
					blockToBeTransferred->next = tempNext;
				}
				blockToBeTransferred = nextBlockToBeTranferred;
			}
			tempFile->data.blocksInFile.head->next = tempFile->data.blocksInFile.tail;
			delete tempFile;
		}
		cout << "FILE DELETED SUCCESSFULLY" << endl << endl;
		mergeBlocksInPool();
	}

	~fileSystem()
	{
		delete[] disk;
		numOfSectors = sizeOfSector = numOfSectorsInPool = 0;

		node<block>*tempBlock = pool.head;
		node<block>*tempBlockNext=nullptr;
		while (tempBlock != nullptr)
		{
			tempBlockNext = tempBlock->next;
			delete tempBlock;
			tempBlock = tempBlockNext;
		}
		delete tempBlock;
		delete tempBlockNext;
	}

	int getSizeOfSector()
	{
		return sizeOfSector;
	}

	int getAvailableSectors()
	{
		return numOfSectorsInPool;
	}

	int getTotalSectors()
	{
		return numOfSectors;
	}
};

void printListOfBlocks(list<block>obj)
{
	node<block>*temp = obj.head;
	while (temp != nullptr)
	{
		cout << "Starting Sector ID: " << temp->data.startingSectorID << endl << "Total Sectors: " << temp->data.totalSectors << endl <<temp->next <<endl<<endl;
		temp = temp->next;
	}
}


int main()
{
	string temp;
	bool flag = true;
	do
	{
		system("cls");
		flag = true;
		cout << "Enter the number of sectors in system: ";
		getline(cin, temp);
		if (temp == "")
		{
			flag = false;
		}
		else
		{
			for (int i = 0; i < temp.length(); i++)
			{
				if (temp[i] < '0' || temp[i] > '9' || temp == "")
				{
					flag = false;
					break;
				}
			}
		}
	} while (flag == false);

	int numOfSectors;
	numOfSectors = stoi(temp);

	flag = true;
	do
	{
		system("cls");
		flag = true;
		cout << "Enter the size of sector: ";
		getline(cin, temp);
		if (temp == "")
		{
			flag = false;
		}
		else
		{
			for (int i = 0; i < temp.length(); i++)
			{
				if (temp[i] < '0' || temp[i] > '9' || temp == "")
				{
					flag = false;
					break;
				}
			}
		}
	} while (flag == false);


	int sizeOfSector;
	sizeOfSector = stoi(temp);

	fileSystem sys(numOfSectors, sizeOfSector);
	int ui=0;

	flag = true;

	while (ui != 5)
	{
		do
		{
			system("cls");
			flag = true;
		
			cout << "Total Sectors: " << sys.getTotalSectors() << endl;
			cout << "Number Of Available Sectors: " << sys.getAvailableSectors();
			cout << endl << "Size of Each Sector: " << sys.getSizeOfSector() << endl << endl;

			cout << "Enter 1 to add a new file." << endl;
			cout << "Enter 2 to delete an existing file." << endl;
			cout << "Enter 3 to read an existing file." << endl;
			cout << "Enter 4 to Read All Files." << endl;
			cout << "Enter 5 to Exit." << endl<<endl;
			//cin.ignore(256, '\n');
			getline(cin, temp);
			if (temp == "")
			{
				flag = false;
			}
			else
			{
				for (int i = 0; i < temp.length(); i++)
				{
					if (temp[i] < '0' || temp[i] > '9' || temp == "")
					{
						flag = false;
						break;
					}
				}
			}
		} while (flag == false);

		
		ui = stoi(temp);


		if (ui == 1)
		{

			string userInput;
			string str;
			userInput.clear();
			bool res;
			cout << "Enter file name: ";
			//cin.ignore(256, '\n');
			getline(cin, userInput);
			//cin >> userInput;
			char*fileName = new char[userInput.length() + 1];
			strcpy_s(fileName,userInput.length()+1, userInput.c_str());


			cout << "Enter the contents of file: ";
			//cin.ignore(256, '\n');
			getline(cin, userInput);
			//cin >> userInput;
			char* fileContent = new char[userInput.length() + 1];
			strcpy_s(fileContent,userInput.length()+1, userInput.c_str());
			res=sys.saveFile(fileName, fileContent, strlen(fileContent));
			if (res == true)
			{
				cout<<"FILE SAVED SUCCESSFULLY"<<endl;
				system("pause");
			}
			else
			{
				cout << "FILE COULD NOT BE SAVED" << endl;
				system("pause");
			}
			continue;
		}
		else if (ui == 2)
		{
			string userInput;
			cout << "Enter file name: ";
			//cin.ignore(256, '\n');
			getline(cin, userInput);
			//cin >> userInput;
			char*fileName = new char[userInput.length() + 1];
			strcpy_s(fileName,userInput.length()+1, userInput.c_str());
			sys.deleteFile(fileName);

			system("pause");

			continue;
		}
		else if (ui == 3)
		{
			string userInput;
			cout << "Enter file name: ";
			//cin.ignore(256, '\n');
			getline(cin, userInput);
			cout << endl << "CONTENTS: ";
			//cin >> userInput;
			char*fileName = new char[userInput.length() + 1];
			strcpy_s(fileName,userInput.length()+1, userInput.c_str());
			sys.readFile(fileName);
			cout << endl << endl << endl;
			system("pause");

			continue;
		}
		else if (ui == 4)
		{
			sys.readAllFiles();
			system("pause");
		}
	}
	system("pause");
}

