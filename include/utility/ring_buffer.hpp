#ifndef RING_BUFFER
#define RING_BUFFER

#include <semaphore>
#include <mutex>
#include <optional>
#include <chrono>
#include <vector>
#include <atomic>
#include <ranges>

template <typename value_type, size_t size>
class ring_buffer
{
private:
	using pointer_type = value_type *;

private:
	std::array<value_type, size> buffer;

	pointer_type read_pointer = &buffer.front();
	pointer_type write_pointer = &buffer.front();

	std::mutex mutable write_mutex;
	std::mutex mutable read_mutex;

	std::atomic<size_t> unread_elements = 0;
	std::atomic<size_t> unwritten_elements = size;

	std::counting_semaphore<size> unread_elements_semaphore{0};
	std::counting_semaphore<size> unwritten_elements_semaphore{size};

public:
	// constructor
	ring_buffer(void) = default;

	// destructor
	~ring_buffer(void) = default;

	// not copyable
	ring_buffer(ring_buffer const &other) = delete;
	auto operator=(ring_buffer const &other) -> ring_buffer & = delete;

	// not copyable
	ring_buffer(ring_buffer &&other) noexcept = default;
	auto operator=(ring_buffer &&other) noexcept -> ring_buffer & = default;

	// single value operations
	auto push(value_type const &value) -> void;
	auto pop(void) -> value_type;

	auto try_push(value_type const &value) -> bool;
	auto try_pop(void) -> std::optional<value_type>;

	template<class Rep, class Period>
	auto try_push_for(value_type const &value, std::chrono::duration<Rep, Period> const constrelative_time) -> bool;

	template<class Rep, class Period>
	auto try_pop_for(std::chrono::duration<Rep, Period> const relative_time) -> std::optional<value_type>;

	// multi value operations
	auto push_all(std::vector<value_type> const &values) -> void;
	auto pop_all(std::vector<value_type> value_buffer = {}) -> std::vector<value_type>;

	auto try_push_all(std::vector<value_type> const &values) -> bool;

	template<class Rep, class Period>
	auto try_push_all_for(std::vector<value_type> const &value, std::chrono::duration<Rep, Period> const relative_time) -> bool;
};

// single value operations
template <typename value_type, size_t size>
auto
ring_buffer<value_type, size>::push(value_type const &value) -> void
{
	// critical write section till end of function
	std::lock_guard<std::mutex> lock{write_mutex};

	unwritten_elements_semaphore.acquire();
	unwritten_elements--;

	*write_pointer = value;

	// set write_pointer to next element
	write_pointer = write_pointer == &buffer.back() ? &buffer.front() : write_pointer + 1;

	// increase counter of unread elements
	unread_elements++;
	unread_elements_semaphore.release();
}

template <typename value_type, size_t size>
auto
ring_buffer<value_type, size>::pop(void) -> value_type
{
	// critical read section till end of function
	std::lock_guard<std::mutex> lock{read_mutex};

	unread_elements_semaphore.acquire();
	unread_elements--;

	value_type value = *read_pointer;

	// set read_pointer to next element
	read_pointer = read_pointer == &buffer.back() ? &buffer.front() : read_pointer + 1;

	// increase counter of written elements
	unwritten_elements++;
	unwritten_elements_semaphore.release();

	return value;
}

template <typename value_type, size_t size>
auto
ring_buffer<value_type, size>::try_push(value_type const &value) -> bool
{
	// critical write section till end of function
	std::lock_guard<std::mutex> lock{write_mutex};

	unwritten_elements_semaphore.acquire();
	if (!unwritten_elements_semaphore.try_acquire())
	{
		return false;
	}
	unwritten_elements--;

	*write_pointer = value;

	// set write_pointer to next element
	write_pointer = write_pointer == &buffer.back() ? &buffer.front() : write_pointer + 1;

	// increase counter of unread elements
	unread_elements++;
	unread_elements_semaphore.release();

	return true;
}

template <typename value_type, size_t size>
auto
ring_buffer<value_type, size>::try_pop() -> std::optional<value_type>
{
	// critical read section till end of function
	std::lock_guard<std::mutex> lock{read_mutex};

	if (!unread_elements_semaphore.try_acquire())
	{
		return std::nullopt;
	}
	unread_elements--;

	value_type value = *read_pointer;

	// set read_pointer to next element
	read_pointer = read_pointer == &buffer.back() ? &buffer.front() : read_pointer + 1;

	// increase counter of written elements
	unwritten_elements++;
	unwritten_elements_semaphore.release();

	return value;
}

template <typename value_type, size_t size>
template<class Rep, class Period>
auto
ring_buffer<value_type, size>::try_push_for(value_type const &value, std::chrono::duration<Rep, Period> const relative_time) -> bool
{
	// critical write section till end of function
	std::lock_guard<std::mutex> lock{write_mutex};

	unwritten_elements_semaphore.acquire();
	if (!unwritten_elements_semaphore.try_acquire_for(relative_time))
	{
		return false;
	}
	unwritten_elements--;

	*write_pointer = value;

	// set write_pointer to next element
	write_pointer = write_pointer == &buffer.back() ? &buffer.front() : write_pointer + 1;

	// increase counter of unread elements
	unread_elements++;
	unread_elements_semaphore.release();

	return true;
}

template <typename value_type, size_t size>
template<class Rep, class Period>
auto
ring_buffer<value_type, size>::try_pop_for(std::chrono::duration<Rep, Period> const relative_time) -> std::optional<value_type>
{
	// critical read section till end of function
	std::lock_guard<std::mutex> lock{read_mutex};

	if (!unread_elements_semaphore.try_acquire_for(relative_time))
	{
		return std::nullopt;
	}
	unread_elements--;

	value_type value = *read_pointer;

	// set read_pointer to next element
	read_pointer = read_pointer == &buffer.back() ? &buffer.front() : read_pointer + 1;

	// increase counter of written elements
	unwritten_elements++;
	unwritten_elements_semaphore.release();

	return value;
}

// multi value operations
template <typename value_type, size_t size>
auto
ring_buffer<value_type, size>::push_all(std::vector<value_type> const &values) -> void
{
	// critical write section till end of function
	std::lock_guard<std::mutex> lock{write_mutex};

	for ([[maybe_unused]] auto _ : values)
	{
		unwritten_elements_semaphore.acquire();
	}

	for (auto const &value : values)
	{
		unwritten_elements--;

		*write_pointer = value;

		// set write_pointer to next element
		write_pointer = write_pointer == &buffer.back() ? &buffer.front() : write_pointer + 1;

		// increase counter of unread elements
		unread_elements++;
		unread_elements_semaphore.release();
	}
}

template <typename value_type, size_t size>
auto
ring_buffer<value_type, size>::pop_all(std::vector<value_type> value_buffer) -> std::vector<value_type>
{
	// critical read section till end of function
	std::lock_guard<std::mutex> lock{read_mutex};

	size_t const value_count = unread_elements;
	for ([[maybe_unused]] auto _ : std::views::iota(static_cast<size_t>(0), value_count))
	{
		unread_elements_semaphore.acquire();
	}

	value_buffer.clear();

	for ([[maybe_unused]] auto _ : std::views::iota(static_cast<size_t>(0), value_count))
	{
		unread_elements--;

		value_buffer.push_back(*read_pointer);

		// set read_pointer to next element
		read_pointer = read_pointer == &buffer.back() ? &buffer.front() : read_pointer + 1;

		// increase counter of written elements
		unwritten_elements++;
		unwritten_elements_semaphore.release();
	}
	return value_buffer;
}

template <typename value_type, size_t size>
auto
ring_buffer<value_type, size>::try_push_all(std::vector<value_type> const &values) -> bool
{
	// critical write section till end of function
	std::lock_guard<std::mutex> lock{write_mutex};

	size_t acquired_semaphores = 0;
	for ([[maybe_unused]] auto _ : values)
	{
		if (!unwritten_elements_semaphore.try_acquire())
		{
			for ([[maybe_unused]] auto _ : std::views::iota(static_cast<size_t>(0), acquired_semaphores))
			{
				unwritten_elements_semaphore.release();
			}
			return false;
		}
		acquired_semaphores++;
	}

	for (auto const &value : values)
	{
		unwritten_elements--;

		*write_pointer = value;

		// set write_pointer to next element
		write_pointer = write_pointer == &buffer.back() ? &buffer.front() : write_pointer + 1;

		// increase counter of unread elements
		unread_elements++;
		unread_elements_semaphore.release();
	}

	return true;
}

template <typename value_type, size_t size>
template<class Rep, class Period>
auto
ring_buffer<value_type, size>::try_push_all_for(std::vector<value_type> const &values, std::chrono::duration<Rep, Period> const relative_time) -> bool
{
	// critical write section till end of function
	std::lock_guard<std::mutex> lock{write_mutex};

	size_t acquired_semaphores = 0;
	for ([[maybe_unused]] auto _ : values)
	{
		if (!unwritten_elements_semaphore.try_acquire_for(relative_time))
		{
			for ([[maybe_unused]] auto _ : std::views::iota(static_cast<size_t>(0), acquired_semaphores))
			{
				unwritten_elements_semaphore.release();
			}
			return false;
		}
		acquired_semaphores++;
	}

	for (auto const &value : values)
	{
		unwritten_elements--;

		*write_pointer = value;

		// set write_pointer to next element
		write_pointer = write_pointer == &buffer.back() ? &buffer.front() : write_pointer + 1;

		// increase counter of unread elements
		unread_elements++;
		unread_elements_semaphore.release();
	}

	return true;
}

#endif /* RING_BUFFER */
