#ifndef DISTANCE_MAP_HPP
#define DISTANCE_MAP_HPP

#include <unordered_map>

// 프레임 404개 기준, 거리값 80m → 10m 선형 감소
const std::unordered_map<int, double> frame_to_distance = {
    {0, 80.0000},
    {1, 79.8263},
    {2, 79.6526},
    {3, 79.4789},
    {4, 79.3052},
    {5, 79.1315},
    {6, 78.9578},
    {7, 78.7841},
    {8, 78.6104},
    {9, 78.4367},
    {10, 78.2630},
    {11, 78.0893},
    {12, 77.9156},
    {13, 77.7419},
    {14, 77.5682},
    {15, 77.3945},
    {16, 77.2208},
    {17, 77.0471},
    {18, 76.8734},
    {19, 76.6998},
    {20, 76.5261},
    {21, 76.3524},
    {22, 76.1787},
    {23, 76.0050},
    {24, 75.8313},
    {25, 75.6576},
    {26, 75.4839},
    {27, 75.3102},
    {28, 75.1365},
    {29, 74.9628},
    {30, 74.7891},
    {31, 74.6154},
    {32, 74.4417},
    {33, 74.2680},
    {34, 74.0943},
    {35, 73.9206},
    {36, 73.7469},
    {37, 73.5732},
    {38, 73.3995},
    {39, 73.2258},
    {40, 73.0521},
    {41, 72.8784},
    {42, 72.7047},
    {43, 72.5310},
    {44, 72.3573},
    {45, 72.1836},
    {46, 72.0099},
    {47, 71.8362},
    {48, 71.6625},
    {49, 71.4888},
    {50, 71.3151},
    {51, 71.1414},
    {52, 70.9677},
    {53, 70.7940},
    {54, 70.6203},
    {55, 70.4467},
    {56, 70.2730},
    {57, 70.0993},
    {58, 69.9256},
    {59, 69.7519},
    {60, 69.5782},
    {61, 69.4045},
    {62, 69.2308},
    {63, 69.0571},
    {64, 68.8834},
    {65, 68.7097},
    {66, 68.5360},
    {67, 68.3623},
    {68, 68.1886},
    {69, 68.0149},
    {70, 67.8412},
    {71, 67.6675},
    {72, 67.4938},
    {73, 67.3201},
    {74, 67.1464},
    {75, 66.9727},
    {76, 66.7990},
    {77, 66.6253},
    {78, 66.4516},
    {79, 66.2779},
    {80, 66.1042},
    {81, 65.9305},
    {82, 65.7568},
    {83, 65.5831},
    {84, 65.4094},
    {85, 65.2357},
    {86, 65.0620},
    {87, 64.8883},
    {88, 64.7146},
    {89, 64.5409},
    {90, 64.3672},
    {91, 64.1935},
    {92, 64.0199},
    {93, 63.8462},
    {94, 63.6725},
    {95, 63.4988},
    {96, 63.3251},
    {97, 63.1514},
    {98, 62.9777},
    {99, 62.8040},
    {100, 62.6303},
    {101, 62.4566},
    {102, 62.2829},
    {103, 62.1092},
    {104, 61.9355},
    {105, 61.7618},
    {106, 61.5881},
    {107, 61.4144},
    {108, 61.2407},
    {109, 61.0670},
    {110, 60.8933},
    {111, 60.7196},
    {112, 60.5459},
    {113, 60.3722},
    {114, 60.1985},
    {115, 60.0248},
    {116, 59.8511},
    {117, 59.6774},
    {118, 59.5037},
    {119, 59.3300},
    {120, 59.1563},
    {121, 58.9826},
    {122, 58.8089},
    {123, 58.6352},
    {124, 58.4615},
    {125, 58.2878},
    {126, 58.1141},
    {127, 57.9404},
    {128, 57.7667},
    {129, 57.5931},
    {130, 57.4194},
    {131, 57.2457},
    {132, 57.0720},
    {133, 56.8983},
    {134, 56.7246},
    {135, 56.5509},
    {136, 56.3772},
    {137, 56.2035},
    {138, 56.0298},
    {139, 55.8561},
    {140, 55.6824},
    {141, 55.5087},
    {142, 55.3350},
    {143, 55.1613},
    {144, 54.9876},
    {145, 54.8139},
    {146, 54.6402},
    {147, 54.4665},
    {148, 54.2928},
    {149, 54.1191},
    {150, 53.9454},
    {151, 53.7717},
    {152, 53.5980},
    {153, 53.4243},
    {154, 53.2506},
    {155, 53.0769},
    {156, 52.9032},
    {157, 52.7295},
    {158, 52.5558},
    {159, 52.3821},
    {160, 52.2084},
    {161, 52.0347},
    {162, 51.8610},
    {163, 51.6873},
    {164, 51.5136},
    {165, 51.3400},
    {166, 51.1663},
    {167, 50.9926},
    {168, 50.8189},
    {169, 50.6452},
    {170, 50.4715},
    {171, 50.2978},
    {172, 50.1241},
    {173, 49.9504},
    {174, 49.7767},
    {175, 49.6030},
    {176, 49.4293},
    {177, 49.2556},
    {178, 49.0819},
    {179, 48.9082},
    {180, 48.7345},
    {181, 48.5608},
    {182, 48.3871},
    {183, 48.2134},
    {184, 48.0397},
    {185, 47.8660},
    {186, 47.6923},
    {187, 47.5186},
    {188, 47.3449},
    {189, 47.1712},
    {190, 46.9975},
    {191, 46.8238},
    {192, 46.6501},
    {193, 46.4764},
    {194, 46.3027},
    {195, 46.1290},
    {196, 45.9553},
    {197, 45.7816},
    {198, 45.6079},
    {199, 45.4342},
    {200, 45.2605},
    {201, 45.0868},
    {202, 44.9132},
    {203, 44.7395},
    {204, 44.5658},
    {205, 44.3921},
    {206, 44.2184},
    {207, 44.0447},
    {208, 43.8710},
    {209, 43.6973},
    {210, 43.5236},
    {211, 43.3499},
    {212, 43.1762},
    {213, 43.0025},
    {214, 42.8288},
    {215, 42.6551},
    {216, 42.4814},
    {217, 42.3077},
    {218, 42.1340},
    {219, 41.9603},
    {220, 41.7866},
    {221, 41.6129},
    {222, 41.4392},
    {223, 41.2655},
    {224, 41.0918},
    {225, 40.9181},
    {226, 40.7444},
    {227, 40.5707},
    {228, 40.3970},
    {229, 40.2233},
    {230, 40.0496},
    {231, 39.8759},
    {232, 39.7022},
    {233, 39.5285},
    {234, 39.3548},
    {235, 39.1811},
    {236, 39.0074},
    {237, 38.8337},
    {238, 38.6600},
    {239, 38.4864},
    {240, 38.3127},
    {241, 38.1390},
    {242, 37.9653},
    {243, 37.7916},
    {244, 37.6179},
    {245, 37.4442},
    {246, 37.2705},
    {247, 37.0968},
    {248, 36.9231},
    {249, 36.7494},
    {250, 36.5757},
    {251, 36.4020},
    {252, 36.2283},
    {253, 36.0546},
    {254, 35.8809},
    {255, 35.7072},
    {256, 35.5335},
    {257, 35.3598},
    {258, 35.1861},
    {259, 35.0124},
    {260, 34.8387},
    {261, 34.6650},
    {262, 34.4913},
    {263, 34.3176},
    {264, 34.1439},
    {265, 33.9702},
    {266, 33.7965},
    {267, 33.6228},
    {268, 33.4491},
    {269, 33.2754},
    {270, 33.1017},
    {271, 32.9280},
    {272, 32.7543},
    {273, 32.5806},
    {274, 32.4069},
    {275, 32.2333},
    {276, 32.0596},
    {277, 31.8859},
    {278, 31.7122},
    {279, 31.5385},
    {280, 31.3648},
    {281, 31.1911},
    {282, 31.0174},
    {283, 30.8437},
    {284, 30.6700},
    {285, 30.4963},
    {286, 30.3226},
    {287, 30.1489},
    {288, 29.9752},
    {289, 29.8015},
    {290, 29.6278},
    {291, 29.4541},
    {292, 29.2804},
    {293, 29.1067},
    {294, 28.9330},
    {295, 28.7593},
    {296, 28.5856},
    {297, 28.4119},
    {298, 28.2382},
    {299, 28.0645},
    {300, 27.8908},
    {301, 27.7171},
    {302, 27.5434},
    {303, 27.3697},
    {304, 27.1960},
    {305, 27.0223},
    {306, 26.8486},
    {307, 26.6749},
    {308, 26.5012},
    {309, 26.3275},
    {310, 26.1538},
    {311, 25.9801},
    {312, 25.8065},
    {313, 25.6328},
    {314, 25.4591},
    {315, 25.2854},
    {316, 25.1117},
    {317, 24.9380},
    {318, 24.7643},
    {319, 24.5906},
    {320, 24.4169},
    {321, 24.2432},
    {322, 24.0695},
    {323, 23.8958},
    {324, 23.7221},
    {325, 23.5484},
    {326, 23.3747},
    {327, 23.2010},
    {328, 23.0273},
    {329, 22.8536},
    {330, 22.6799},
    {331, 22.5062},
    {332, 22.3325},
    {333, 22.1588},
    {334, 21.9851},
    {335, 21.8114},
    {336, 21.6377},
    {337, 21.4640},
    {338, 21.2903},
    {339, 21.1166},
    {340, 20.9429},
    {341, 20.7692},
    {342, 20.5955},
    {343, 20.4218},
    {344, 20.2481},
    {345, 20.0744},
    {346, 19.9007},
    {347, 19.7270},
    {348, 19.5533},
    {349, 19.3797},
    {350, 19.2060},
    {351, 19.0323},
    {352, 18.8586},
    {353, 18.6849},
    {354, 18.5112},
    {355, 18.3375},
    {356, 18.1638},
    {357, 17.9901},
    {358, 17.8164},
    {359, 17.6427},
    {360, 17.4690},
    {361, 17.2953},
    {362, 17.1216},
    {363, 16.9479},
    {364, 16.7742},
    {365, 16.6005},
    {366, 16.4268},
    {367, 16.2531},
    {368, 16.0794},
    {369, 15.9057},
    {370, 15.7320},
    {371, 15.5583},
    {372, 15.3846},
    {373, 15.2109},
    {374, 15.0372},
    {375, 14.8635},
    {376, 14.6898},
    {377, 14.5161},
    {378, 14.3424},
    {379, 14.1687},
    {380, 13.9950},
    {381, 13.8213},
    {382, 13.6476},
    {383, 13.4739},
    {384, 13.3002},
    {385, 13.1266},
    {386, 12.9529},
    {387, 12.7792},
    {388, 12.6055},
    {389, 12.4318},
    {390, 12.2581},
    {391, 12.0844},
    {392, 11.9107},
    {393, 11.7370},
    {394, 11.5633},
    {395, 11.3896},
    {396, 11.2159},
    {397, 11.0422},
    {398, 10.8685},
    {399, 10.6948},
    {400, 10.5211},
    {401, 10.3474},
    {402, 10.1737},
    {403, 10.0000}
};

#endif  // DISTANCE_MAP_HPP
