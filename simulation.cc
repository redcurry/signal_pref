// Numeric simulation of evolving sexual organisms
// to investigate signal and preference

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>

using namespace std;


#define N 100     // Population size

// Mutation rates (per genome per generation)
#define U_PREF_POS     1
#define U_PREF_VALUE   1
#define U_SIGNAL_POS   1
#define U_SIGNAL_VALUE 1

#define INIT_PREF_POS 0.5
#define INIT_PREF_VALUE 0.5

#define INIT_SIGNAL_POS 0.5
#define INIT_SIGNAL_VALUE 0.5

#define LOVE_SLOPE -8

#define POS_MUT_EFFECT 0.01
#define VALUE_MUT_EFFECT 0.1

#define UPDATES 10000000   // Number of replication attempts to perform


struct organism
{
  float pref_pos;
  float pref_value;

  float signal_pos;
  float signal_value;

  int generation;

  void print() { cout << pref_pos << ", " << pref_value << ", "
                      << signal_pos << ", " << signal_value << endl; }
};


// Population functions
vector<organism> create_population();
void print_header();
void print_info(vector<organism> const & pop);
vector<int> get_generations(vector<organism> const & pop);
vector<double> get_pref_pos(vector<organism> const & pop);
vector<double> get_pref_values(vector<organism> const & pop);
vector<double> get_signal_pos(vector<organism> const & pop);
vector<double> get_signal_values(vector<organism> const & pop);

// Reproduction functions
void replicate_next_organism(vector<organism> & pop);
bool should_recombine(organism const & parent_1, organism const & parent_2);
double prob_love(organism const & parent_1, organism const & parent_2);
organism recombine(organism const & parent_1, organism const & parent_2);
bool is_inside(double pos, double start, double end);
double mutate_pos(double pos);
double mutate_value(double value);

// Random functions
int random_pos();
double rand_0_to_1();

// General list functions
template<typename T> void print(vector<T> list);
template<typename T> double mean(vector<T> const & list);


int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    cout << "Arguments: random_seed" << endl;
    return EXIT_FAILURE;
  }

  // Initialize command-line arguments
  int random_seed = atoi(argv[1]);

  srand(random_seed);

  vector<organism> pop = create_population();
  
  print_header();

  for(int update = 0; update < UPDATES; update++)
  {
    replicate_next_organism(pop);

    if (update % 1000 == 0)
      print_info(pop);
  }

  //print(get_pref_pos(pop));

/*
  organism parent1;
  parent1.pref_pos = rand_0_to_1();
  parent1.pref_value = rand_0_to_1();
  parent1.signal_pos = rand_0_to_1();
  parent1.signal_value = rand_0_to_1();
  parent1.generation = 0;

  organism parent2;
  parent2.pref_pos = rand_0_to_1();
  parent2.pref_value = rand_0_to_1();
  parent2.signal_pos = rand_0_to_1();
  parent2.signal_value = rand_0_to_1();
  parent2.generation = 0;

  parent1.print();
  parent2.print();

  organism recombinant = recombine(parent1, parent2);
  recombinant.print();
*/
  return 0;
}


vector<organism> create_population()
{
  return vector<organism>(N, (organism)
    {INIT_PREF_POS, INIT_PREF_VALUE, INIT_SIGNAL_POS, INIT_SIGNAL_VALUE, 0});
}


void print_header()
{
  cout << "MeanGeneration MeanPrefPos MeanPrefValue "
    << "MeanSignalPos MeanSignalValue\n";
}


void replicate_next_organism(vector<organism> & pop)
{
  int parent_1_pos = random_pos();
  int parent_2_pos = random_pos();

  int child_pos = random_pos();

  organism parent_1 = pop[parent_1_pos];
  organism parent_2 = pop[parent_2_pos];

  // Do nothing if either organism doesn't have a pref or signal pos
  if (parent_1.pref_pos < 0 || parent_1.signal_pos < 0
      || parent_2.pref_pos < 0 || parent_2.signal_pos < 0)
    return;

  // Do nothing if signal and preference don't match
  if (!should_recombine(parent_1, parent_2))
    return;

  organism recombinant = recombine(parent_1, parent_2);
  for (int i = 0; i < 100; i++)
    recombinant = recombine(recombinant, recombinant);

  // Do nothing if recombinant didn't inherit pref or signal
  if (recombinant.pref_pos < 0 || recombinant.signal_pos < 0)
    return;

  pop[child_pos] = recombinant;

  if (rand_0_to_1() < U_PREF_POS)
    pop[child_pos].pref_pos = mutate_pos(pop[child_pos].pref_pos);

  if (rand_0_to_1() < U_PREF_VALUE)
    pop[child_pos].pref_value = mutate_value(pop[child_pos].pref_value);

  if (rand_0_to_1() < U_SIGNAL_POS) 
    pop[child_pos].signal_pos = mutate_pos(pop[child_pos].signal_pos);

  if (rand_0_to_1() < U_SIGNAL_VALUE)
    pop[child_pos].signal_value = mutate_value(pop[child_pos].signal_value);

  // Not sure what the right calculation should be here
  pop[child_pos].generation = parent_1.generation + 1;
}


int random_pos()
{
  return rand() % N;
}


bool should_recombine(organism const & parent_1, organism const & parent_2)
{
  double prob_1_loves_2 = prob_love(parent_1, parent_2);
  double prob_2_loves_1 = prob_love(parent_2, parent_1);
  return rand_0_to_1() < prob_1_loves_2 * prob_2_loves_1;
}


double prob_love(organism const & parent_1, organism const & parent_2)
{
  double abs_diff = abs(parent_1.pref_value - parent_2.signal_value);
  double prob = LOVE_SLOPE * abs_diff + 1;
  return prob > 0.0 ? prob : 0.0;
}


organism recombine(organism const & parent_1, organism const & parent_2)
{
  double crossover_start = rand_0_to_1();
  double crossover_size = rand_0_to_1();

  double crossover_end = crossover_start + crossover_size;
  if (crossover_end > 1)
    crossover_end -= 1;
/*
  cout << "crossover: " << crossover_start << ", " << crossover_end
    << ", size: " << crossover_size << endl;
*/
  organism recombinant;
  recombinant.pref_pos = -1;      // sentinel that pref wasn't inherited
  recombinant.signal_pos = -1;    // sentinel that signal wasn't inherited

  if (is_inside(parent_1.pref_pos, crossover_start, crossover_end))
  {
    recombinant.pref_pos = parent_1.pref_pos;
    recombinant.pref_value = parent_1.pref_value;
  }

  if (is_inside(parent_1.signal_pos, crossover_start, crossover_end))
  {
    recombinant.signal_pos = parent_1.signal_pos;
    recombinant.signal_value = parent_1.signal_value;
  }

  if (!is_inside(parent_2.pref_pos, crossover_start, crossover_end))
  {
    recombinant.pref_pos = parent_2.pref_pos;
    recombinant.pref_value = parent_2.pref_value;
  }

  if (!is_inside(parent_2.signal_pos, crossover_start, crossover_end))
  {
    recombinant.signal_pos = parent_2.signal_pos;
    recombinant.signal_value = parent_2.signal_value;
  }

  // Force them to be equal
  //recombinant.signal_value = recombinant.pref_value;

  return recombinant;
}


bool is_inside(double pos, double start, double end)
{
  if (start < end)
    return pos > start && pos < end;
  else
    return pos > start || pos < end;
}


double rand_0_to_1()
{
  return (double)rand() / RAND_MAX;
}


double mutate_pos(double pos)
{

  double new_pos = pos + (rand_0_to_1() - 0.5) * POS_MUT_EFFECT;
  if (new_pos < 0.0)
    return new_pos + 1;
  else if (new_pos > 1)
    return new_pos - 1;
  return new_pos;

//  return new_pos < 0 ? 0 : (new_pos >= 1 ? 0.9999 : new_pos);
//  return rand_0_to_1();
}


double mutate_value(double value)
{
  return value + (rand_0_to_1() - 0.5) * VALUE_MUT_EFFECT;
//  return rand_0_to_1();
}


template<typename T>
T sum(vector<T> const & list)
{
  T sum = 0;
  for(int i = 0; i < list.size(); i++)
    sum += list[i];
  return sum;
}


void print_info(vector<organism> const & pop)
{
  cout << mean(get_generations(pop)) << " "
    << mean(get_pref_pos(pop)) << " "
    << mean(get_pref_values(pop)) << " "
    << mean(get_signal_pos(pop)) << " "
    << mean(get_signal_values(pop)) << endl;
}


template<typename T>
double mean(vector<T> const & list)
{
  return (double)sum(list) / list.size();
}


vector<int> get_generations(vector<organism> const & pop)
{
  vector<int> generations;
  for (int i = 0; i < pop.size(); i++)
    generations.push_back(pop[i].generation);
  return generations;
}


vector<double> get_pref_pos(vector<organism> const & pop)
{
  vector<double> pref_pos;
  for (int i = 0; i < pop.size(); i++)
    pref_pos.push_back(pop[i].pref_pos);
  return pref_pos;
}


vector<double> get_pref_values(vector<organism> const & pop)
{
  vector<double> pref_values;
  for (int i = 0; i < pop.size(); i++)
    pref_values.push_back(pop[i].pref_value);
  return pref_values;
}


vector<double> get_signal_pos(vector<organism> const & pop)
{
  vector<double> signal_pos;
  for (int i = 0; i < pop.size(); i++)
    signal_pos.push_back(pop[i].signal_pos);
  return signal_pos;
}


vector<double> get_signal_values(vector<organism> const & pop)
{
  vector<double> signal_values;
  for (int i = 0; i < pop.size(); i++)
    signal_values.push_back(pop[i].signal_value);
  return signal_values;
}


template<typename T>
void print(vector<T> list)
{
  for (int i = 0; i < list.size(); i++)
    cout << list[i] << endl;
}
