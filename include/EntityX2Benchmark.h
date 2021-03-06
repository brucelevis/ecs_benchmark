#ifndef ENTITYX2BENCHMARK_H_
#define ENTITYX2BENCHMARK_H_

#include <string>
#include <vector>
#include <memory>
#include <random>
#include <numeric>
#include <functional>

#include "entityx/entityx.hh"

class EntityX2Benchmark {
    public:

    struct PositionComponent {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct DirectionComponent {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct ComflabulationComponent {
        float thingy = 0.0;
        int dingy = 0;
        bool mingy = false;
        std::string stringy;
    };


    // Convenience types for our entity system.
    using GameComponents = entityx::Components<
        PositionComponent, DirectionComponent, ComflabulationComponent
    >;

    static constexpr size_t INITIAL_CAPACITY = 2097152L; // bignumber to avoid alloc error, benchmark a lot of enteties
    using EntityManager = entityx::EntityX<GameComponents, entityx::ColumnStorage<GameComponents, INITIAL_CAPACITY>>;

    template <typename C>
    using Component = EntityManager::Component<C>;

    using Entity = EntityManager::Entity;

    typedef double TimeDelta;



    class System {
        public:
        // virtual dtor and the rule of 6
        System() = default;
        virtual ~System() = default;
        System(const System&) = default;
        System& operator=(const System&) = default;
        System(System&&) = default;
        System& operator=(System&&) = default;

        virtual void update(EntityManager& es, TimeDelta dt) = 0;
    };


    class MovementSystem : public System {
        public:
        MovementSystem() = default;

        void update(EntityManager& es, TimeDelta dt) override {
            Component<PositionComponent> position;
            Component<DirectionComponent> direction;

            for (auto entity : es.entities_with_components<PositionComponent, DirectionComponent>(position, direction)) {
                position->x += direction->x * dt;
                position->y += direction->y * dt;
            }
        }
    };

    class ComflabSystem : public System {
        public:
        ComflabSystem() = default;

        void update(EntityManager& es, TimeDelta dt) override {
            Component<ComflabulationComponent> comflab;

            for (auto entity : es.entities_with_components<ComflabulationComponent>(comflab)) {
                comflab->thingy *= 1.000001f;
                comflab->mingy = !comflab->mingy;
                comflab->dingy++;
                //comflab.stringy = std::to_string(comflab.dingy);
            }
        }
    };

    #ifdef USE_MORECOMPLEX_SYSTEM
    class MoreComplexSystem : public System {
        private:
        int random(int min, int max){
            // Seed with a real random value, if available
            static std::random_device r;
        
            // Choose a random mean between min and max
            static std::default_random_engine e1 (r());

            std::uniform_int_distribution<int> uniform_dist(min, max);

            return uniform_dist(e1);
        }

        public:
        MoreComplexSystem() = default;

        void update(EntityManager& es, TimeDelta dt) override {
            Component<PositionComponent> position;
            Component<DirectionComponent> direction;
            Component<ComflabulationComponent> comflab;

            for (auto entity : es.entities_with_components(comflab, direction, comflab)) {
                if(comflab) {
                    std::vector<double> vec;
                    for(size_t i = 0;i < comflab->dingy && i < 100;i++){
                        vec.push_back(i * comflab->thingy);
                    }

                    int sum = std::accumulate(std::begin(vec), std::end(vec), 0.0);
                    int product = std::accumulate(std::begin(vec), std::end(vec), 1, std::multiplies<double>());

                    comflab->stringy = std::to_string(comflab->dingy);

                    if(position && direction && comflab->dingy % 10000 == 0) {
                        if(position->x > position->y) {
                            direction->x = random(0, 5);
                            direction->y = random(0, 10);
                        } else {
                            direction->x = random(0, 10);
                            direction->y = random(0, 5);
                        }
                    }
                }
            }
        }
    };
    #endif

    class Application {
        public:
        Application() {
            this->systems_.emplace_back(std::make_unique<MovementSystem>());
            this->systems_.emplace_back(std::make_unique<ComflabSystem>());
            #ifdef USE_MORECOMPLEX_SYSTEM
            this->systems_.emplace_back(std::make_unique<MoreComplexSystem>());
            #endif
        }

        void update(TimeDelta dt) {
            for (auto& system : this->systems_) {
                system->update(this->entities_, dt);
            }
        }

        EntityManager& getEntityManager() { return this->entities_; }
        const EntityManager& getEntityManager() const { return this->entities_; }

        private:
        EntityManager entities_;
        std::vector<std::unique_ptr<System>> systems_;
    };

    static constexpr TimeDelta fakeDeltaTime = 1.0 / 60;
};

#endif // ENTITYX2BENCHMARK_H_