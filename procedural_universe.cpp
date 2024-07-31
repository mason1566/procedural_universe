#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <random>

constexpr uint32_t starColours[8] = {
    0xFFFFFFFF, 0xFFD9FFFF, 0xFFA3FFFF, 0xFFFFC8C8,
    0xFFFFCB9D, 0xFF9F9FFF, 0xFF415EFF, 0xFF28199D
};

struct Planet {
    double distance = 0.0;
    double diameter = 0.0;
    double temperature = 0.0;
    bool ring = false;
    std::vector<double> moons;
};

class StarSystem 
{
public:
    StarSystem(uint32_t x, uint32_t y, bool generateFullSystem = false) {
        nLehmer = (x & 0xFFFF) << 16 | (y & 0xFFFF);

        starExists = (rndInt(0, 20) == 1);
        if (!starExists) return; // Star bouncer

        starDiameter = rndDouble(10.0, 40.0);
        starColour.n = starColours[rndInt(0, 8)];

        if (!generateFullSystem) return;

        double distanceFromStar = rndDouble(60.0, 200.0);
        int nPlanets = rndInt(0, 10);
        for (int i = 0; i < nPlanets; i++) {
            Planet p;
            p.distance = distanceFromStar;
            distanceFromStar += rndDouble(20.0, 200.0);
            p.diameter = rndDouble(4.0, 20.0);
            p.temperature = rndDouble(-200.0, 300.0);
            p.ring = rndInt(0, 10) == 1;

            int nMoons = std::max(rndInt(-5, 5), 0); // approx 50/50 that a planet has > 0 moons
            for (int n = 0; n < nMoons; n++) {
                p.moons.push_back(rndDouble(1.0, 5.0)); 
            }
            planets.push_back(p);
        }
    }

    // VARIABLES
    bool starExists = false;
    double starDiameter = 0.0f;
    olc::Pixel starColour = olc::WHITE;
    std::vector<Planet> planets;


private:
    uint32_t nLehmer = 0;
    uint32_t Lehmer32() {
        nLehmer += 0xe120fc15;
        u_int64_t tmp;
        tmp = (uint64_t)nLehmer * 0x4a39b70d;
        uint32_t m1 = (tmp >> 32) ^ tmp;
        tmp = (uint64_t)m1 * 0x12fad5c9;
        uint32_t m2 = (tmp >> 32) ^ tmp;
        return m2;
    }

    int rndInt(int min, int max) {
        return (Lehmer32() % (max-min)) + min;
    }

    double rndDouble(double min, double max) {
        return ((double)Lehmer32() / (double)(0x7FFFFFFF)) * (max - min) + min;
    }
};

class ProceduralUniverse : public olc::PixelGameEngine 
{
public:
    ProceduralUniverse() {
        sAppName = "procedural_universe";
    }

    olc::vf2d galaxyOffset = { 0,0 }; // camera location
    bool bStarSelected = false;
    olc::vi2d vStarSelected = { 0,0 };

    bool OnUserCreate() override {
        return true;
    }

    bool OnUserUpdate(float elapsedTime) override {
        if (GetKey(olc::W).bHeld) galaxyOffset.y -= 50.0f * elapsedTime; // multiply by elapsedTime to keep time consistent in-game
        if (GetKey(olc::S).bHeld) galaxyOffset.y += 50.0f * elapsedTime;
        if (GetKey(olc::A).bHeld) galaxyOffset.x -= 50.0f * elapsedTime;
        if (GetKey(olc::D).bHeld) galaxyOffset.x += 50.0f * elapsedTime;
        
        Clear(olc::BLACK);
        int sectorsCountX = ScreenWidth() / 16;
        int sectorsCountY = ScreenHeight() / 16;

        olc::vi2d mouse = { GetMouseX() / 16, GetMouseY() / 16 };
        olc::vi2d galaxy_mouse = mouse + galaxyOffset;

        olc::vi2d screen_sector = { 0,0 };

        for (screen_sector.x = 0; screen_sector.x < sectorsCountX; screen_sector.x++) {
            for (screen_sector.y = 0; screen_sector.y < sectorsCountY; screen_sector.y++) 
            {
                StarSystem star(screen_sector.x + (uint32_t)galaxyOffset.x, screen_sector.y + (uint32_t)galaxyOffset.y);

                if (star.starExists) {
                    FillCircle(screen_sector.x * 16 + 8, screen_sector.y * 16 + 8, (int)star.starDiameter / 8, star.starColour);

                    if (mouse.x == screen_sector.x && mouse.y == screen_sector.y) {
                        DrawCircle(screen_sector.x * 16 + 8, screen_sector.y * 16 + 8, 12, olc::YELLOW);
                    }
                }
            }
        }

        if (GetMouse(0).bPressed) // left-click
        {
            StarSystem star(galaxy_mouse.x, galaxy_mouse.y);
            if (star.starExists) {
                bStarSelected = true;
                vStarSelected = galaxy_mouse;
            }
            else bStarSelected = false;
        }

        if (bStarSelected) 
        {
            StarSystem star(vStarSelected.x, vStarSelected.y, true);

            // Draw Window
            FillRect(8, 240, 496, 232, olc::DARK_BLUE);
            DrawRect(8, 240, 496, 232, olc::WHITE);

            // Draw Star
            olc::vi2d vBody = { 14, 356 };
            vBody.x += star.starDiameter * 1.375;
            FillCircle(vBody, (int)(star.starDiameter * 1.375), star.starColour);
            vBody.x += (star.starDiameter * 1.375) + 8;

            
        }
        
        return true;
    }
};

int main() {
    ProceduralUniverse demo;
    if (demo.Construct(512, 480, 2, 2)) demo.Start();
    return 0;
}