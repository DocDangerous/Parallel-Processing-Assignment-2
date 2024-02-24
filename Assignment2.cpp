#include <iostream>
#include <thread>
#include <queue>
#include <cmath>
#include <chrono>
#include <fstream>
#include <mutex>
#include <vector>
#include <random>

bool cupcake_present = true;
bool game_afoot = true;
int invite = 0;
int num_guests_p1 = 0;
std::vector<bool> maze_visited;
std::mutex vector_mutex;
std::mutex maze_mutex;
std::mutex game_mutex;
std::mutex invite_mutex;
std::mutex queue_mutex;
std::mutex vase_mutex;
std::queue<int> p2_queue;
std::random_device rd;
std::mt19937 mt(rd());

//Mutex wrapper to randomly select the next guest to enter
void next_invite()
{
    std::lock_guard<std::mutex> lock(invite_mutex);
    std::uniform_int_distribution<int> dist(0, num_guests_p1-1);
    invite = dist(mt);
    std::cout << "Guest" << invite << " has been invited in!" << std::endl;
}

int check_invite()
{
    std::lock_guard<std::mutex> lock(invite_mutex);
    return invite;
}

//Mutex wrapper to check is game is ongoing
bool is_game_afoot()
{
   std::lock_guard<std::mutex> lock(game_mutex);
   return game_afoot;
}

//Announce to the minotaur everyone has been through
void end_game()
{
    std::lock_guard<std::mutex> lock(game_mutex);
    game_afoot = false;
}

//Mutex wrapper for boolean visited array. Will only ever update to true
void update_vector(int guest_id)
{
    std::lock_guard<std::mutex> lock(vector_mutex);
    maze_visited[guest_id] = true;
}

/*Guests wait at the mutex lock to simulate waiting for an invitation to enter.
When a guest enters the maze they eat the cupcake the first time they see it, otherwise
they leave the plate as it is regardless of cupcake-ness. The guest of id 0 has been specifically
charged before hand with replacing and counting the cupcakes. Each time he goes in the maze, he calls for a new
one if the plate is empty and counts it, otherwise he lets the plate be. He knows the number of guests going in,
so he needs to go through the maze AT LEAST num_guests times, until he has placed num_guests
cupcakes on the plate. Once he's done so, he announces that every guest has been through the maze.
*/
void enter_maze(int id, bool* visited, int* cupcakes)
{
    std::cout << "Guest" << id << " has entered the maze!" << std::endl;

    if (id == 0) //Guest 0 behavior
    {
        if(!*visited) //First time behavior
        {
            (*cupcakes)++; //For RP: eat the cupcake, replace it.
            std::cout << "Cupcakes placed: " << *cupcakes << std::endl;
        }
        if (!cupcake_present) { //Place the cupcake if there was one and count it
            cupcake_present = true;
            (*cupcakes)++;
            std::cout << "Cupcakes placed: " << *cupcakes << std::endl;
        }
    }
    else //Everyone else's behavior
    {
        if (*cupcakes > 0) return; //Been here before and eaten a cupcake? Leave untouched.
        if (cupcake_present){ //First time seeing a cupcake
            cupcake_present = false; //Ask for replacement if no cupcake
            (*cupcakes)++;
            std::cout << "Guest " << id << " has eaten a cupcake!" << std::endl;
        }
    }

    if (!*visited)
    {
        *visited = true; //Mark self as having been here before
        update_vector(id);
    }

    return;
}

//Behavior for each guest in the labyrinth
void problem1ThreadBehavior(int guest_id, int num_guests)
{
    bool visited_already = false;
    int cupcakes = 0; //Confusing name. For guest 0, this is # cupcakes placed. Everyone else, this is cupcakes eaten.
    bool announcer = false;

    //Guest 0 has been chosen before the game to be the counter. See enter_maze() comment
    if (guest_id == 0) {
        announcer = true;
    }

    while(game_afoot)
    {
        if (check_invite() == guest_id) //Check if you've been invited
        {
            std::lock_guard<std::mutex> lock(maze_mutex);
            enter_maze(guest_id, &visited_already, &cupcakes);

            if (announcer && (cupcakes >= num_guests)){
                std::cout << "Guest" << guest_id << " has announced the game is over!" << std::endl;
                end_game();
                return;
            }
            else {
                std::uniform_int_distribution<int> dist(0, num_guests_p1-1);
                invite = dist(mt);
                std::cout << "Guest" << invite << " has been invited in!" << std::endl;
            }
        }
    }
}

void problem1()
{
    std::cout << "Starting problem 1!" << std::endl;
    while(true)
    {
        std::cout << "Enter number of guests: ";
        std::cin >> num_guests_p1;
        if ((num_guests_p1 > 0) && (num_guests_p1 < INT_MAX)) break;
        std::cout << "Let's be real here. Try again." << std::endl;
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < num_guests_p1; i++) {
        threads.emplace_back(problem1ThreadBehavior, i, num_guests_p1);
        maze_visited.emplace_back(false);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    //Count to see if every guest had actually been through once the game is over
    bool victory = true;
    int id = 0;
    for (auto const j : maze_visited) {
        victory = j;
        id++;
        if (!victory) std::cout << "Guest " << id << " has not been through the maze!" << std::endl;
    }
    if (victory) std::cout << "Everyone has been through the maze!" << std::endl;

    std::cout << "Finished problem 1!" << std::endl;
    return;
}

//Guests try to insert their id into the queue without breaking the queue object
void enqueue_id(int id)
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    p2_queue.emplace(id);
    std::cout << "Guest #" << id << " has entered the queue!" <<std::endl;

    return;
}

//Thread-safe way to check front of queue
int get_front_id()
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (!p2_queue.empty())
    {
        return p2_queue.front();
    }
    return -1; //Queue is empty
}

//Mutex wrapper to see if queue is empty
bool queue_empty()
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    return p2_queue.empty();
}

//Iterate through the queue and see if the given id is still waiting in it
bool is_id_in_queue(int id)
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    std::queue<int> clone = p2_queue;
    while(!clone.empty())
    {
        if (clone.front() == id) return true;
        clone.pop();
    }
    return false;
}

void inform_front()
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (!p2_queue.empty())
    {
        p2_queue.pop();
    }
    return;
}
//Enter the showroom, then tell the next guest they can enter the showroom.
void view_vase(int id)
{
    //Having a separate mutex gives the program time to have stuff actually process in parallel
    std::lock_guard<std::mutex> lock(vase_mutex);
    std::cout << "Guest #" << id << " has started viewing the vase!" <<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Guest #" << id << " has finished viewing the vase!" <<std::endl;
    return;
}

//Determines whether or not a guest wants to enter the queue, based on a "percentage" likelihood.
bool wants_to_queue(int likelihood)
{
    std::uniform_int_distribution<int> dist(1, 100);
    int rand = dist(mt);
    if (rand <= likelihood) return true;
    return false;
}

//Function simulating a guest in problem 2
void problem2ThreadBehavior(int guest_id)
{
    //Decide if they want to view the vase, 80% likelihood
    bool wanting_to_view = wants_to_queue(80);
    while(wanting_to_view)
    {
        bool waiting_in_queue = true;
        enqueue_id(guest_id);
        if (get_front_id() == guest_id) // Base/initial case
        {
            //First in line, no one to tell you you can dequeue, so walk in
            view_vase(guest_id);
            inform_front();
            inform_front(); //Call it twice because this guest was in queue too
            waiting_in_queue = false;
        }
        while(waiting_in_queue)
        {
            //Wait until you've been told you can enter the showroom
            if (!is_id_in_queue(guest_id))
            {
                view_vase(guest_id); //Enter the showroom and close the door
                inform_front(); //Tell the next guest it's their turn
                waiting_in_queue = false;
            }
        }
        wanting_to_view = wants_to_queue(20); //20% chance to go back in line
    }
    std::cout << "Guest #" << guest_id << " has lost interest!" <<std::endl;
    get_front_id();
    return;
}

void problem2()
{
    int num_guests = 1;
    std::cout << "Starting problem 2!" << std::endl;
    while(true)
    {
        std::cout << "Note: This problem uses a sort of sleep() function to emulate " << std::endl;
        std::cout << "occupancy of the showroom. A large number of guests will work but might take a while! " << std::endl;
        std::cout << "Enter number of guests: ";
        std::cin >> num_guests;
        if ((num_guests > 0) && (num_guests < INT_MAX)) break;
        std::cout << "Let's be real here. Try again." << std::endl;
    }

    //Make the threads and give them their respective id
    std::vector<std::thread> threads;
    for (int i = 0; i < num_guests; i++) {
        threads.emplace_back(problem2ThreadBehavior, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Finished problem 2!" << std::endl;
    return;
}

int main()
{
    problem1();
    problem2();
    return 0;
}
