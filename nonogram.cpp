#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using std::cout;
using std::endl;
using std::ifstream;
using std::ws;
using std::string;
using std::getline;
using std::istringstream;
using std::vector;

static void read_spec(const string &spec, vector<int> &output){
  istringstream spec_reader(spec);
  int t;
  do{
    spec_reader >> t;
    if(spec_reader.fail()){break;}
    output.push_back(t);
  } while(true);
}

struct board_view{
  int len;
  char **board_status;
  int pos;
  bool is_row;

  board_view(int _len, char **_board_status, int _pos, bool _is_row) : len(_len), board_status(_board_status), pos(_pos), is_row(_is_row) {}

  char operator[](int idx) const{
    if(is_row){return board_status[pos][idx];} else {return board_status[idx][pos];}
  }
};

char **cell_possibility[2];
bool *place_check[2];

//Here we use a dynamic programming approach to determine the possibilities of each cell.
static void check_possibility(const board_view &view, const vector<int> &spec, char *output){
  if(spec.size() == 0){
    for(int i = 0;i < view.len;++i){output[i] = 2;}
    return;
  }
  
  //Now assume there is at least one element in spec.
  for(int i = 0;i < view.len;++i){
    place_check[0][i] = false;
    place_check[1][i] = false;
    for(int j = 0;j < view.len;++j){
      cell_possibility[0][i][j] = 0;
      cell_possibility[1][i][j] = 0;
    }
  }
  
  int size = spec.size();
  for(int i = size - 1;i >= 0;--i){
    int t = spec[i];
    for(int j = view.len - 1;j >= 0;--j){
      if(j + t > view.len){continue;}
      if(i < size - 1 && j + t + 1 >= view.len){continue;}
      bool check = true;
      for(int k = 0;k < t;++k){
	if(view[j + k] == 2){check = false;break;}
      }
      if(check){
	if(i == size - 1){
	  for(int k = j + t;k < view.len;++k){
	    if(view[k] == 1){check = false;break;}
	  }
	  if(check){
	    place_check[1][j] = true;
	    for(int k = 0;k < t;++k){
	      cell_possibility[1][j][j + k] = 1;
	    }
	    for(int k = j + t;k < view.len;++k){
	      cell_possibility[1][j][k] = 2;
	    }
	  }
	} else {
	  if(view[j + t] != 1){
	    for(int k = j + t + 1;k < view.len;++k){
	      if(place_check[0][k]){
		place_check[1][j] = true;
		for(int l = 0;l < t;++l){
		  cell_possibility[1][j][j + l] |= 1;
		}
		for(int l = j + t;l < k;++l){
		  cell_possibility[1][j][l] |= 2;
		}
		for(int l = k;l < view.len;++l){
		  cell_possibility[1][j][l] |= cell_possibility[0][k][l];
		}
	      }
	      if(view[k] == 1){break;}
	    }
	  }
	}
      }
      if(view[j] != 1 && j < view.len - 1 && place_check[1][j + 1]){
	place_check[1][j] = true;
	cell_possibility[1][j][j] |= 2;
	for(int k = j + 1;k < view.len;++k){
	  cell_possibility[1][j][k] |= cell_possibility[1][j + 1][k];
	}
      }
    }
    for(int j = 0;j < view.len;++j){
      place_check[0][j] = place_check[1][j];
      place_check[1][j] = false;
      for(int k = 0;k < view.len;++k){
	cell_possibility[0][j][k] = cell_possibility[1][j][k];
	cell_possibility[1][j][k] = 0;
      }
    }
  }
  if(place_check[0][0]){
    for(int i = 0;i < view.len;++i){
      output[i] = cell_possibility[0][0][i];
    }
  } else {
    for(int i = 0;i < view.len;++i){
      output[i] = 0;
    }
  }
}

struct board_snapshot{
  char **board_status;
  int assump_x, assump_y;
  int row, col;

  board_snapshot(const char* const* _board_status, int _row, int _col, int _assump_x, int _assump_y) : board_status(NULL), assump_x(_assump_x), assump_y(_assump_y), row(_row), col(_col) {
    board_status = new char*[row];
    for(int i = 0;i < row;++i){
      board_status[i] = new char[col];
      for(int j = 0;j < col;++j){
	board_status[i][j] = _board_status[i][j];
      }
    }
  }

  ~board_snapshot(){
    for(int i = 0;i < row;++i){
      delete [] board_status[i];
    }
    delete [] board_status;
  }
};

int main(int argc, char** argv){
  if(argc != 2){
    cout << "Usage: ./nonogram filename" << endl;
    return 0;
  }
  ifstream fin(argv[1]);
  int row, col;
  fin >> row >> col >> ws;
  char **board_status = new char*[row];
  //0 -> Indeterminate, 1 -> Must black, 2 -> Must white, 3 -> Inconsistency
  for(int i = 0;i < row;++i){board_status[i] = new char[col];}
  vector<int> *row_specs, *col_specs;
  row_specs = new vector<int>[row];
  col_specs = new vector<int>[col];
  string spec;
  for(int i = 0;i < row;++i){
    getline(fin, spec);
    if(!fin.good()){cout << "Invalid input" << endl; return 0;}
    read_spec(spec, row_specs[i]);
  }
  for(int i = 0;i < col;++i){
    getline(fin, spec);
    if(!fin.good()){cout << "Invalid input" << endl; return 0;}
    read_spec(spec, col_specs[i]);
  }

  for(int i = 0;i < row;++i){
    for(int j = 0;j < col;++j){
      board_status[i][j] = 0;
    }
  }

  bool updated;
  int max_dim = row > col ? row : col;
  char *buffer = new char[max_dim];
  bool *row_updated, *col_updated, *row_updated_new, *col_updated_new;
  row_updated = new bool[row];
  col_updated = new bool[col];
  row_updated_new = new bool[row];
  col_updated_new = new bool[col];
  cell_possibility[0] = new char*[max_dim];
  cell_possibility[1] = new char*[max_dim];
  place_check[0] = new bool[max_dim];
  place_check[1] = new bool[max_dim];
  for(int i = 0;i < max_dim;++i){
    cell_possibility[0][i] = new char[max_dim];
    cell_possibility[1][i] = new char[max_dim];
  }

  int recursion_depth = 0;
  vector<board_snapshot*> board_stack;

 attempt_solve:
  for(int i = 0;i < row;++i){row_updated[i] = true;}
  for(int i = 0;i < col;++i){col_updated[i] = true;}
  do{
    updated = false;
    for(int i = 0;i < row;++i){row_updated_new[i] = false;}
    for(int i = 0;i < col;++i){col_updated_new[i] = false;}
    
    for(int i = 0;i < row;++i){
      if(!row_updated[i]){continue;}
      check_possibility(board_view(col, board_status, i, true), row_specs[i], buffer);
      for(int j = 0;j < col;++j){
	if(board_status[i][j] != 0){continue;}
	switch(buffer[j]){
	case 0: {
	  //cout << "Inconsistency found!" << endl;
	  if(recursion_depth == 0){return 0;}
	  board_snapshot *last_snapshot = *(board_stack.rbegin());
	  for(int k = 0;k < row;++k){
	    for(int l = 0;l < col;++l){
	      board_status[k][l] = last_snapshot->board_status[k][l];
	    }
	  }
	  board_status[last_snapshot->assump_x][last_snapshot->assump_y] = 2;
	  delete last_snapshot;
	  board_stack.pop_back();
	  --recursion_depth;
	  goto attempt_solve;
	}
	case 1:
	  board_status[i][j] = 1;
	  updated = true;
	  row_updated_new[i] = true;
	  col_updated_new[j] = true;
	  break;
	case 2:
	  board_status[i][j] = 2;
	  updated = true;
	  row_updated_new[i] = true;
	  col_updated_new[j] = true;
	  break;
	case 3:
	  break;
	}
      }
    }
    
    for(int i = 0;i < col;++i){
      if(!col_updated[i]){continue;}
      check_possibility(board_view(row, board_status, i, false), col_specs[i], buffer);
      for(int j = 0;j < row;++j){
	if(board_status[j][i] != 0){continue;}
	switch(buffer[j]){
	case 0: {
	  //cout << "Inconsistency found!" << endl;
	  if(recursion_depth == 0){return 0;}
	  board_snapshot *last_snapshot = *(board_stack.rbegin());
	  for(int k = 0;k < row;++k){
	    for(int l = 0;l < col;++l){
	      board_status[k][l] = last_snapshot->board_status[k][l];
	    }
	  }
	  board_status[last_snapshot->assump_x][last_snapshot->assump_y] = 2;
	  delete last_snapshot;
	  board_stack.pop_back();
	  --recursion_depth;
	  goto attempt_solve;
	}
	case 1:
	  board_status[j][i] = 1;
	  updated = true;
	  row_updated_new[j] = true;
	  col_updated_new[i] = true;
	  break;
	case 2:
	  board_status[j][i] = 2;
	  updated = true;
	  row_updated_new[j] = true;
	  col_updated_new[i] = true;
	  break;
	case 3:
	  break;
	}
      }
    }

    for(int i = 0;i < row;++i){row_updated[i] = row_updated_new[i];}
    for(int i = 0;i < col;++i){col_updated[i] = col_updated_new[i];}
  } while(updated);

  bool completion_check = true;
  int prob_row = -1, prob_col = -1;
  for(int i = 0;i < row;++i){
    for(int j = 0;j < col;++j){
      if(board_status[i][j] == 0){
	completion_check = false;
	prob_row = i;
	prob_col = j;
	break;
      }
    }
    if(!completion_check){break;}
  }
  if(!completion_check){
    //cout << "Assuming row " << prob_row << " column " << prob_col << " is filled" << endl;
    ++recursion_depth;
    board_stack.push_back(new board_snapshot(board_status, row, col, prob_row, prob_col));
    board_status[prob_row][prob_col] = 1;
    goto attempt_solve;
  }

  cout << ' ';
  for(int i = 0;i < col;++i){
    if((i + 1) % 5 == 0){cout << 'X';} else {cout << ' ';}
  }
  cout << endl;
  for(int i = 0;i < row;++i){
    if((i + 1) % 5 == 0){cout << 'X';} else {cout << ' ';}
    for(int j = 0;j < col;++j){
      if(board_status[i][j] == 1){
	cout << "■";
      } else if(board_status[i][j] == 2){
	cout << "□";
      } else {
	cout << '?';
      }
    }
    cout << endl;
  }
  return 0;
}
