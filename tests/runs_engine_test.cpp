#include <doctest/doctest.h>
#include <vagabond/core/RunsEngine.h>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// Exposes the protected ticket interface the way MultiEngine uses it: one
// thread publishes scores for tickets of many keys, handing each ticket to
// that key's own reader, which then looks it up. The hand-off itself is
// synchronised (as MultiEngine's per-key mutex is), but the readers share no
// lock with the thread still inserting the other keys' tickets.
struct TicketHolder : public RunsEngine
{
	int next() { return getNextTicket(); }
	void put(int ticket, double score) { setScoreForTicket(ticket, score); }
	float get(int ticket) { return scoreForTicket(ticket); }
	float take(int ticket) { return takeScoreForTicket(ticket); }
};

namespace
{
struct Slot
{
	std::mutex m;
	std::condition_variable cv;
	std::queue<int> tickets;
	bool done = false;
};
}

TEST_CASE("RunsEngine: scores can be read while others are being added")
{
	TicketHolder holder;
	const int readers = 8;
	const int rounds = 4000;

	std::vector<Slot> slots(readers);
	std::mutex result_mutex;
	long wrong = 0;
	long looked_up = 0;

	std::vector<std::thread> threads;
	for (int r = 0; r < readers; r++)
	{
		threads.emplace_back([&, r]()
		{
			Slot &slot = slots[r];
			while (true)
			{
				int ticket = 0;
				{
					std::unique_lock<std::mutex> lock(slot.m);
					slot.cv.wait(lock, [&]()
					             { return !slot.tickets.empty() || slot.done; });
					if (slot.tickets.empty()) { return; }
					ticket = slot.tickets.front();
					slot.tickets.pop();
				}

				float score = holder.get(ticket);
				std::unique_lock<std::mutex> lock(result_mutex);
				looked_up++;
				if (score != (float)ticket) { wrong++; }
			}
		});
	}

	for (int i = 0; i < rounds; i++)
	{
		for (int r = 0; r < readers; r++)
		{
			int ticket = holder.next();
			holder.put(ticket, ticket);
			Slot &slot = slots[r];
			std::unique_lock<std::mutex> lock(slot.m);
			slot.tickets.push(ticket);
			slot.cv.notify_one();
		}
	}

	for (Slot &slot : slots)
	{
		std::unique_lock<std::mutex> lock(slot.m);
		slot.done = true;
		slot.cv.notify_all();
	}

	for (std::thread &t : threads) { t.join(); }

	CHECK(looked_up == (long)readers * rounds);
	CHECK(wrong == 0);
}

TEST_CASE("RunsEngine: a taken score is forgotten, a looked-up one is kept")
{
	TicketHolder holder;
	int kept = holder.next();
	int taken = holder.next();
	holder.put(kept, 2.5);
	holder.put(taken, 4.5);

	CHECK(holder.get(kept) == 2.5f);
	CHECK(holder.get(kept) == 2.5f);
	CHECK(holder.take(taken) == 4.5f);
	// gone now, so reported as missing
	CHECK(holder.take(taken) == FLT_MAX);
	CHECK(holder.get(kept) == 2.5f);
}

TEST_CASE("RunsEngine: copies keep the scores but not the lock")
{
	TicketHolder holder;
	int ticket = holder.next();
	holder.put(ticket, 1.5);

	TicketHolder copy = holder;
	CHECK(copy.get(ticket) == 1.5f);
	copy.put(copy.next(), 3.5);
	CHECK(copy.get(ticket + 1) == 3.5f);
}
