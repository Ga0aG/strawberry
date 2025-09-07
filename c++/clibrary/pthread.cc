#include <pthread.h>
#include <unistd.h> // For sleep function
#include "../include/helper.hh"

std::vector<std::string> mgs = {
  "Tick Tock 1",
  "Tick Tock 2",
};

// Function to be executed by each thread
void *printNumbers(void *id)
{
  int thread_id = *((int *)id);
  for (int i = 0; i < 2; ++i)
  {
    INFO_STREAM("Thread" << thread_id << " prints: " << mgs[i]);
    sleep(1); // Sleep for 1 second to simulate work
  }
  return nullptr;
}

int main()
{
  const int NUM_THREADS = 3;      // Number of threads
  pthread_t threads[NUM_THREADS]; // Array to hold thread IDs
  int threadIds[NUM_THREADS];     // Array to hold thread IDs for passing to threads

  // Create threads
  for (int i = 0; i < NUM_THREADS; ++i)
  {
    threadIds[i] = i + 1; // Assign thread ID
    int result = pthread_create(&threads[i], nullptr, printNumbers, (void *)&threadIds[i]);
    if (result)
    {
      std::cerr << "Error creating thread " << i << ": " << result << std::endl;
      return 1;
    }
  }

  // Wait for all threads to complete
  for (int i = 0; i < NUM_THREADS; ++i)
  {
    pthread_join(threads[i], nullptr);
  }

  INFO_STREAM("All threads completed.");
  return 0;
}
