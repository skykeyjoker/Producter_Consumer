#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <mutex>

static const int MAX_SUMS = 300;
static int sums = 0;

std::mutex sCoutMutex; // 输出锁，维护输出格式正常

std::condition_variable convConsumer, convProduter;

class Producter
{
public:
    Producter(int efficiency) : mEfficiency(efficiency) {}

    void product()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::unique_lock lock(mMutex);
        convProduter.wait(lock, [this] {
            return sums <= MAX_SUMS;
        });
        sums += mEfficiency;

        {
            std::lock_guard coutLock(sCoutMutex);
            std::cout << "Producter " << std::this_thread::get_id()
                      << " producted " << mEfficiency
                      << "g, Now there are " << sums << "g" << std::endl;
        }

        convConsumer.notify_all();
    }

    void startProduct()
    {
        while (true)
        {
            product();
        }
    }

private:
    int mEfficiency;
    std::mutex mMutex;
};

class Consumer
{
public:
    Consumer(int efficiency) : mEfficiency(efficiency) {}

    void consume()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));

        std::unique_lock lock(mMutex);
        convConsumer.wait(lock, [this] {
            return sums > mEfficiency;
        });
        sums -= mEfficiency;

        {
            std::lock_guard coutLock(sCoutMutex);
            std::cout << "Consumer " << std::this_thread::get_id()
                      << " consumed " << mEfficiency
                      << "g, Now there are " << sums << "g" << std::endl;
        }

        convProduter.notify_one();
    }

    void startConsume()
    {
        while (true)
        {
            consume();
        }
    }

private:
    int mEfficiency;
    std::mutex mMutex;
};

int main()
{
    Producter p1(60);
    Producter p2(30);

    Consumer c1(20);
    Consumer c2(30);
    Consumer c3(10);

    std::thread tp1(&Producter::startProduct, std::ref(p1));
    std::thread tp2(&Producter::startProduct, std::ref(p2));

    std::thread tc1(&Consumer::startConsume, std::ref(c1));
    std::thread tc2(&Consumer::startConsume, std::ref(c2));
    std::thread tc3(&Consumer::startConsume, std::ref(c3));

    tp1.join();
    tp2.join();
    tc1.join();
    tc2.join();
    tc3.join();

    return 0;
}