import concurrent.futures
import time

# Function to simulate a task that takes some time
def task(n):
    print(f'Starting task {n}')
    time.sleep(2)  # Simulate a delay
    result = n * n
    print(f'Finished task {n}')
    return result

# Main function to use the thread pool
def main():
    numbers = [1, 2, 3, 4, 5]  # List of numbers to process
    results = []

    # Using ThreadPoolExecutor to manage threads.
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as executor:
        # Submit tasks to the thread pool
        future_to_number = {executor.submit(task, num): num for num in numbers}

        # Collect results as they are completed
        for future in concurrent.futures.as_completed(future_to_number):
            num = future_to_number[future]
            try:
                result = future.result()
                results.append(result)
                print(f'Result for task {num}: {result}')
            except Exception as exc:
                print(f'Task {num} generated an exception: {exc}')
        # executor.shutdown()

    print('All tasks completed.')
    print('Results:', results)

if __name__ == '__main__':
    main()
