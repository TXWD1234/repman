#include "Project.hpp"

int main(int argc, char* argv[]) {
	std::fs::path dataPath(std::getenv("HOME"));
	dataPath = dataPath / ".local/share/repman/data.json";
	if (!std::fs::exists(dataPath)) {
		std::fs::create_directories(dataPath.parent_path());
		std::ofstream ofs(dataPath, std::ios::binary);
		ofs.write("{\n}", 3);
		ofs.close();
	}
	tx::JsonObject root = tx::parseJson(tx::readWholeFileText(dataPath));
	if (argc <= 1) { // no arg
		// list all entries
		for (int i = 0; i < root.size(); i++) {
			cout << '"' << root.atIndex(i).k() << '"' << ": \"" << root.atIndex(i).v().get<string>() << "\"\n";
		}
	} else {
		if (root.exist(argv[1])) {
			string cmd = "code \"";
			cmd.append(root[argv[1]].get<string>());
			cmd.push_back('"');
			cout << cmd << endl;
			system(cmd.c_str());
		} else {
			cout << "entry not found.\n";
		}
	}
	return 0;
}

// things to add:
// 1. txjson: add append
// 2. add append functionality
// 3. txjson: add iterator