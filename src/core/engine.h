#ifndef FLINT_ENGINE_H
#define FLINT_ENGINE_H

#include <chrono>

namespace Flint {
    enum class Signal {
        SwapChainChanged,
        RebuildCommandBuffer,
    };

    class Engine {
    public:
        static Engine &getSingleton() {
            static Engine singleton;

            return singleton;
        }

        Engine();

        void tick();

        double get_delta() const;

        double get_elapsed() const;

    private:
        double elapsed = 0;
        double delta = 0;
    };
}

#endif //FLINT_ENGINE_H