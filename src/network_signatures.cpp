#include "network_signatures.h"

network_signatures :: network_signatures()
{
    signatures.clear();
}

void network_signatures :: create_signature_for_graph(
                            computation_graph & CG,
                            region_constraints & input_region,
                            uint32_t no_of_samples)
{
  int samples_counter;
  map<uint32_t, double > current_sample;
  bit_vector b_vec;

  samples_counter = 0;
  while(samples_counter < no_of_samples)
  {
    if(input_region.return_sample(current_sample, 100 + 29 * samples_counter))
    {
      CG.evaluate_graph(current_sample, b_vec);
      signatures[samples_counter] = b_vec;
      b_vec.clear();
    }
    samples_counter ++;
  }

}

void network_signatures :: clear()
{
  signatures.clear();
}

bool network_signatures ::empty()
{
  return signatures.empty();
}


vector< uint32_t > network_signatures :: return_sample_indices()
{
  vector<uint32_t> index_vector;
  for(auto each_signature : signatures)
  {
    index_vector.push_back(each_signature.first);
  }
  return index_vector;
}

void network_signatures :: return_bit_vector_for_sample(uint32_t index, bit_vector & b_vec)
{
  assert(signatures.find(index) != signatures.end());
  b_vec = signatures[index];
}

uint32_t trial_percent = 1000;
void network_signatures :: learn_constant_neurons(
                            set < uint32_t > & on_list,
                            set < uint32_t > & off_list)

{
  on_list.clear();
  off_list.clear();

  bit_vector b_vec;
  uint32_t bit_index;
  bool first_time = true;

  for(auto some_index : signatures.return_sample_indices())
  {
    signatures.return_bit_vector_for_sample(some_index, b_vec);
    if(first_time)
    // basically iniitialize the list of 'on' neuron with all the neurons which are
    // on initially, and likewise for the 'off' neurons
    {
      first_time = false;
      for(auto each_bit : b_vec)
      {
        if(each_bit.second)
        {
          on_list.insert(each_bit.first);
        }
        else
        {
          off_list.insert(each_bit.first);
        }
      }

    }
    else
    {
      auto size = b_vec.size();
      for(auto each_bit : b_vec)
      {
        if(each_bit.second)
        // if neuron is 'on' find it in the off list and remove it
        {
          if(off_list.find(each_bit.first) != off_list.end());
          {
            off_list.erase(off_list.find(each_bit.first)) ;
          }
        }
        else
        // if neuron is 'off' find it in the on list and remove it
        {
          if(on_list.find(each_bit.first) != on_list.end());
          {
            on_list.erase(on_list.find(each_bit.first)) ;
          }
        }
      }
    }
  }



}

void network_signatures :: learn_pairwise_relationship(uint32_t trial_count,
                                 set< pair< uint32_t, uint32_t > >& same_sense_nodes,
                                 set< pair< uint32_t, uint32_t > >& opposite_sense_nodes)

{
  assert(!signatures.empty());

  same_sense_nodes.clear();
  opposite_sense_nodes.clear();
  uint32_t random_index;

  for(auto some_signature : signatures)
  {
    random_index = some_signature.first;
    break;
  }

  auto number_of_neurons = signatures[random_index];

  auto trial_index = 0;
  while(trial_index < trial_count)
  {
    uint32_t  neuron_1_index = generate_random_int(number_of_neurons, trial_index * 17);
    uint32_t  neuron_2_index = generate_random_int(number_of_neurons, trial_index * 17 + 13);

    bool is_same_sense = true;
    bool is_opposite_sense = true;

    auto current_pair;
    if(neuron_1_index != neuron_2_index)
    {
      current_pair = make_pair(neuron_1_index, neuron_2_index);

      for(auto each_signature  : signatures)
      {
        if(
           each_signature.second[current_pair.first]
              ==
           each_signature.second[current_pair.second]
         )
        {
          is_opposite_sense = false;
        }
        else
        {
          is_same_sense = false;
        }
      }

      if(is_same_sense)
        same_sense_nodes.insert(current_pair);
      else if(is_opposite_sense)
        opposite_sense_nodes.insert(current_pair);
    }

    trial_index++;
  }

}

void network_signatures :: learn_implies_relationship(
          uint32_t trial_count,
          set< pair< uint32_t, uint32_t > >& node_1_implies_node_2_true_sense,
          set< pair< uint32_t, uint32_t > >& node_1_implies_node_2_false_sense)
// Checking whether things of the nature neuron 1 being 'on' implies neuron 2 is 'on'
// n1 (implies) n2, is basically n2 >= n1 {in C++ terms, !n1 || n2}
// or the other way around like neuron 1 being 'off' implies neuron 2 is  'off'
// (not)n1 (implies) (not)n2, is basically n2 <= n1 {in C++ terms, n1 || !n2 }
{

  assert(!signatures.empty());
  node_1_implies_node_2_true_sense.clear();
  node_1_implies_node_2_false_sense.clear();

  uint32_t random_index;
  for(auto some_signature : signatures)
  {
    random_index  = some_signature.first;
    break;
  }

  uint32_t number_of_neurons = signatures[random_index];
  uint32_t trial_index = 0;
  while(trial_index < trial_count)
  {
    uint32_t neuron_1_index = generate_random_int(number_of_neurons, trial_index* 29 + 23);
    uint32_t neuron_2_index = generate_random_int(number_of_neurons, trial_index* 29 + 17);

    bool n1_imples_n2 = true;
    bool not_n1_implies_not_n2 = true;
    pair< uint32_t, uint32_t > current_pair;
    if(neuron_1_index != neuron_2_index)
    {
      current_pair = make_pair(neuron_1_index, neuron_2_index);
      for(auto each_signature : signatures)
      {
        if(
           ! ( (!each_signature.second[current_pair.first]) ||
           (each_signature.second[current_pair.second]) ) )
           {
             n1_implies_n2 = false;
           }
        if(
            !((each_signature.second[current_pair.first]) ||
            !(each_signature.second[current_pair.second]) )
          )
          {
             not_n1_implies_not_n2 = false;
          }
      }

      if(n1_implies_n2)
        node_1_implies_node_2_true_sense.insert(current_pair);
      if(not_n1_implies_not_n2)
        node_1_implies_node_2_false_sense.insert(current_pair);
    }
    trial_index++;
  }

}



uint32_t generate_random_int(uint32_t range, uint32_t seed)
{
  srand (seed);
  auto random_number = rand() % range;
  return random_number;
}
