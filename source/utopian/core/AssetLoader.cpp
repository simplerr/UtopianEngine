#include "AssetLoader.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/StaticModel.h"

namespace Utopian
{
   AssetLoader& gAssetLoader()
   {
      return AssetLoader::Instance();
   }

   AssetLoader::AssetLoader()
   {
      // Grass
      AddAsset(FLOWER_BOUNCING_BET_01_1, "Grass/Models/flower_bouncing_bet_01_1.fbx", "Grass/Models/Textures/T_flower_bouncing_bet_01_A_M.tga");
      AddAsset(FLOWER_BOUNCING_BET_01_2, "Grass/Models/flower_bouncing_bet_01_2.fbx", "Grass/Models/Textures/T_flower_bouncing_bet_01_A_M.tga");
      AddAsset(FLOWER_BOUNCING_BET_01_CROSS_1, "Grass/Models/flower_bouncing_bet_01_cross_1.fbx", "Grass/Models/Textures/T_flower_bouncing_bet_01_A_M.tga");
      AddAsset(FLOWER_BOUNCING_BET_01_DETAILED_1, "Grass/Models/flower_bouncing_bet_01_detailed_1.fbx", "Grass/Models/Textures/T_flower_bouncing_bet_01_A_M.tga");
      AddAsset(FLOWER_BROWNRAY_KNAPWEED_01_1, "Grass/Models/flower_brownray_knapweed_01_1.fbx", "Grass/Models/Textures/T_flower_brownray_knapweed_01_A_M.tga");
      AddAsset(FLOWER_BROWNRAY_KNAPWEED_01_2, "Grass/Models/flower_brownray_knapweed_01_2.fbx", "Grass/Models/Textures/T_flower_brownray_knapweed_01_A_M.tga");
      AddAsset(FLOWER_BROWNRAY_KNAPWEED_01_CROSS_1, "Grass/Models/flower_brownray_knapweed_01_cross_1.fbx", "Grass/Models/Textures/T_flower_brownray_knapweed_01_A_M.tga");
      AddAsset(FLOWER_BROWNRAY_KNAPWEED_01_DETAILED_1, "Grass/Models/flower_brownray_knapweed_01_detailed_1.fbx", "Grass/Models/Textures/T_flower_brownray_knapweed_01_A_M.tga");
      AddAsset(FLOWER_BROWNRAY_KNAPWEED_01_DETAILED_2, "Grass/Models/flower_brownray_knapweed_01_detailed_2.fbx", "Grass/Models/Textures/T_flower_brownray_knapweed_01_A_M.tga");
      AddAsset(FLOWER_CHAMOMILE_01_1, "Grass/Models/flower_chamomile_01_1.fbx", "Grass/Models/Textures/T_flower_chamomile_01_A_M.tga");
      AddAsset(FLOWER_CHAMOMILE_01_2, "Grass/Models/flower_chamomile_01_2.fbx", "Grass/Models/Textures/T_flower_chamomile_01_A_M.tga");
      AddAsset(FLOWER_CHAMOMILE_01_CROSS_1, "Grass/Models/flower_chamomile_01_cross_1.fbx", "Grass/Models/Textures/T_flower_chamomile_01_A_M.tga");
      AddAsset(FLOWER_CHAMOMILE_01_DETAILED_1, "Grass/Models/flower_chamomile_01_detailed_1.fbx", "Grass/Models/Textures/T_flower_chamomile_01_A_M.tga");
      AddAsset(FLOWER_CHAMOMILE_01_DETAILED_2, "Grass/Models/flower_chamomile_01_detailed_2.fbx", "Grass/Models/Textures/T_flower_chamomile_01_A_M.tga");
      AddAsset(FLOWER_COMMON_CHICORY_01_1, "Grass/Models/flower_common_chicory_01_1.fbx", "Grass/Models/Textures/T_flower_common_chicory_01_A_M.tga");
      AddAsset(FLOWER_COMMON_CHICORY_01_2, "Grass/Models/flower_common_chicory_01_2.fbx", "Grass/Models/Textures/T_flower_common_chicory_01_A_M.tga");
      AddAsset(FLOWER_COMMON_CHICORY_01_CROSS_1, "Grass/Models/flower_common_chicory_01_cross_1.fbx", "Grass/Models/Textures/T_flower_common_chicory_01_A_M.tga");
      AddAsset(FLOWER_COMMON_CHICORY_01_DETAILED_1, "Grass/Models/flower_common_chicory_01_detailed_1.fbx", "Grass/Models/Textures/T_flower_common_chicory_01_A_M.tga");
      AddAsset(FLOWER_COMMON_CHICORY_01_DETAILED_2, "Grass/Models/flower_common_chicory_01_detailed_2.fbx", "Grass/Models/Textures/T_flower_common_chicory_01_A_M.tga");
      AddAsset(FLOWER_COMMON_POPPY_01_1, "Grass/Models/flower_common_poppy_01_1.fbx", "Grass/Models/Textures/T_flower_common_poppy_A_M.tga");
      AddAsset(FLOWER_COMMON_POPPY_01_2, "Grass/Models/flower_common_poppy_01_2.fbx", "Grass/Models/Textures/T_flower_common_poppy_A_M.tga");
      AddAsset(FLOWER_COMMON_POPPY_01_CROSS_1, "Grass/Models/flower_common_poppy_01_cross_1.fbx", "Grass/Models/Textures/T_flower_common_poppy_A_M.tga");
      AddAsset(FLOWER_COMMON_POPPY_01_DETAILED_1, "Grass/Models/flower_common_poppy_01_detailed_1.fbx", "Grass/Models/Textures/T_flower_common_poppy_A_M.tga");
      AddAsset(FLOWER_COMMON_POPPY_01_DETAILED_2, "Grass/Models/flower_common_poppy_01_detailed_2.fbx", "Grass/Models/Textures/T_flower_common_poppy_A_M.tga");
      AddAsset(FLOWER_COMMON_SAINT_JOHNS_WORT_01_1, "Grass/Models/flower_common_Saint_Johns_wort_01_1.fbx", "Grass/Models/Textures/T_flower_common_Saint_Johns_wort_01_A_M.tga");
      AddAsset(FLOWER_COMMON_SAINT_JOHNS_WORT_01_2, "Grass/Models/flower_common_Saint_Johns_wort_01_2.fbx", "Grass/Models/Textures/T_flower_common_Saint_Johns_wort_01_A_M.tga");
      AddAsset(FLOWER_COMMON_SAINT_JOHNS_WORT_01_CROSS_1, "Grass/Models/flower_common_Saint_Johns_wort_01_cross_1.fbx", "Grass/Models/Textures/T_flower_common_Saint_Johns_wort_01_A_M.tga");
      AddAsset(FLOWER_COMMON_SAINT_JOHNS_WORT_01_DETAILED_1, "Grass/Models/flower_common_Saint_Johns_wort_01_detailed_1.fbx", "Grass/Models/Textures/T_flower_common_Saint_Johns_wort_01_A_M.tga");
      AddAsset(FLOWER_CORNFLOWER_01_1, "Grass/Models/flower_cornflower_01_1.fbx", "Grass/Models/Textures/T_flower_cornflower_01_A_M.tga");
      AddAsset(FLOWER_CORNFLOWER_01_2, "Grass/Models/flower_cornflower_01_2.fbx", "Grass/Models/Textures/T_flower_cornflower_01_A_M.tga");
      AddAsset(FLOWER_CORNFLOWER_01_CROSS_1, "Grass/Models/flower_cornflower_01_cross_1.fbx", "Grass/Models/Textures/T_flower_cornflower_01_A_M.tga");
      AddAsset(FLOWER_CORNFLOWER_01_DETAILED_1, "Grass/Models/flower_cornflower_01_detailed_1.fbx", "Grass/Models/Textures/T_flower_cornflower_01_A_M.tga");
      AddAsset(FLOWER_CORNFLOWER_01_DETAILED_2, "Grass/Models/flower_cornflower_01_detailed_2.fbx", "Grass/Models/Textures/T_flower_cornflower_01_A_M.tga");
      AddAsset(FLOWER_GOLDENROD_01_1, "Grass/Models/flower_goldenrod_01_1.fbx", "Grass/Models/Textures/T_flower_goldenrod_01_A_M.tga");
      AddAsset(FLOWER_GOLDENROD_01_2, "Grass/Models/flower_goldenrod_01_2.fbx", "Grass/Models/Textures/T_flower_goldenrod_01_A_M.tga");
      AddAsset(FLOWER_GOLDENROD_01_3, "Grass/Models/flower_goldenrod_01_3.fbx", "Grass/Models/Textures/T_flower_goldenrod_01_A_M.tga");
      AddAsset(FLOWER_GOLDENROD_01_CROSS_1, "Grass/Models/flower_goldenrod_01_cross_1.fbx", "Grass/Models/Textures/T_flower_goldenrod_01_A_M.tga");
      AddAsset(FLOWER_GOLDENROD_01_CROSS_2, "Grass/Models/flower_goldenrod_01_cross_2.fbx", "Grass/Models/Textures/T_flower_goldenrod_01_A_M.tga");
      AddAsset(FLOWER_GOLDENROD_01_DETAILED_1, "Grass/Models/flower_goldenrod_01_detailed_1.fbx", "Grass/Models/Textures/T_flower_goldenrod_01_A_M.tga");
      AddAsset(FLOWER_GOLDENROD_01_DETAILED_2, "Grass/Models/flower_goldenrod_01_detailed_2.fbx", "Grass/Models/Textures/T_flower_goldenrod_01_A_M.tga");
      AddAsset(FLOWER_SUNROOT_01_1, "Grass/Models/flower_sunroot_01_1.fbx", "Grass/Models/Textures/T_flower_sunroot_01_A_M.tga");
      AddAsset(FLOWER_SUNROOT_01_2, "Grass/Models/flower_sunroot_01_2.fbx", "Grass/Models/Textures/T_flower_sunroot_01_A_M.tga");
      AddAsset(FLOWER_SUNROOT_01_CROSS_1, "Grass/Models/flower_sunroot_01_cross_1.fbx", "Grass/Models/Textures/T_flower_sunroot_01_A_M.tga");
      AddAsset(FLOWER_SUNROOT_01_DETAILED_1, "Grass/Models/flower_sunroot_01_detailed_1.fbx", "Grass/Models/Textures/T_flower_sunroot_01_A_M.tga");
      AddAsset(FLOWER_SUNROOT_01_DETAILED_2, "Grass/Models/flower_sunroot_01_detailed_2.fbx", "Grass/Models/Textures/T_flower_sunroot_01_A_M.tga");
      AddAsset(GRASS_MEADOW_01_1, "Grass/Models/grass_meadow_01_1.fbx", "Grass/Models/Textures/T_grass_meadow_01_A_M.tga");
      AddAsset(GRASS_MEADOW_01_2, "Grass/Models/grass_meadow_01_2.fbx", "Grass/Models/Textures/T_grass_meadow_01_A_M.tga");
      AddAsset(GRASS_MEADOW_01_3, "Grass/Models/grass_meadow_01_3.fbx", "Grass/Models/Textures/T_grass_meadow_01_A_M.tga");
      AddAsset(GRASS_MEADOW_01_4, "Grass/Models/grass_meadow_01_4.fbx", "Grass/Models/Textures/T_grass_meadow_01_A_M.tga");
      AddAsset(GRASS_MEADOW_01_5, "Grass/Models/grass_meadow_01_5.fbx", "Grass/Models/Textures/T_grass_meadow_01_A_M.tga");
      AddAsset(GRASS_MEADOW_01_6, "Grass/Models/grass_meadow_01_6.fbx", "Grass/Models/Textures/T_grass_meadow_01_A_M.tga");
      AddAsset(GRASS_MEADOW_01_CROSS_1, "Grass/Models/grass_meadow_01_cross_1.fbx", "Grass/Models/Textures/T_grass_meadow_01_A_M.tga");
      AddAsset(GRASS_MEADOW_01_CROSS_2, "Grass/Models/grass_meadow_01_cross_2.fbx", "Grass/Models/Textures/T_grass_meadow_01_A_M.tga");
      AddAsset(GRASS_MEADOW_01_CROSS_3, "Grass/Models/grass_meadow_01_cross_3.fbx", "Grass/Models/Textures/T_grass_meadow_01_A_M.tga");
      AddAsset(GRASS_MEADOW_01_DETAILED_1, "Grass/Models/grass_meadow_01_detailed_1.fbx", "Grass/Models/Textures/T_grass_meadow_01_A_M.tga");
      AddAsset(GRASS_MEADOW_01_DETAILED_2, "Grass/Models/grass_meadow_01_detailed_2.fbx", "Grass/Models/Textures/T_grass_meadow_01_A_M.tga");
      AddAsset(GRASS_MEADOW_02_1, "Grass/Models/grass_meadow_02_1.fbx", "Grass/Models/Textures/T_grass_meadow_02_A_M.tga", "Grass/Models/Textures/T_grass_meadow_02_N.png");
      AddAsset(GRASS_MEADOW_02_2, "Grass/Models/grass_meadow_02_2.fbx", "Grass/Models/Textures/T_grass_meadow_02_A_M.tga", "Grass/Models/Textures/T_grass_meadow_02_N.png");
      AddAsset(GRASS_MEADOW_02_3, "Grass/Models/grass_meadow_02_3.fbx", "Grass/Models/Textures/T_grass_meadow_02_A_M.tga", "Grass/Models/Textures/T_grass_meadow_02_N.png");
      AddAsset(GRASS_MEADOW_02_4, "Grass/Models/grass_meadow_02_4.fbx", "Grass/Models/Textures/T_grass_meadow_02_A_M.tga", "Grass/Models/Textures/T_grass_meadow_02_N.png");
      AddAsset(GRASS_MEADOW_02_5, "Grass/Models/grass_meadow_02_5.fbx", "Grass/Models/Textures/T_grass_meadow_02_A_M.tga", "Grass/Models/Textures/T_grass_meadow_02_N.png");
      AddAsset(GRASS_MEADOW_02_6, "Grass/Models/grass_meadow_02_6.fbx", "Grass/Models/Textures/T_grass_meadow_02_A_M.tga", "Grass/Models/Textures/T_grass_meadow_02_N.png");
      AddAsset(GRASS_MEADOW_02_CROSS_1, "Grass/Models/grass_meadow_02_cross_1.fbx", "Grass/Models/Textures/T_grass_meadow_02_A_M.tga", "Grass/Models/Textures/T_grass_meadow_02_N.png");
      AddAsset(GRASS_MEADOW_02_CROSS_2, "Grass/Models/grass_meadow_02_cross_2.fbx", "Grass/Models/Textures/T_grass_meadow_02_A_M.tga"), "Grass/Models/Textures/T_grass_meadow_02_N.png";
      AddAsset(GRASS_MEADOW_02_CROSS_3, "Grass/Models/grass_meadow_02_cross_3.fbx", "Grass/Models/Textures/T_grass_meadow_02_A_M.tga", "Grass/Models/Textures/T_grass_meadow_02_N.png");
      AddAsset(GRASS_MEADOW_02_DETAILED_1, "Grass/Models/grass_meadow_02_detailed_1.fbx", "Grass/Models/Textures/T_grass_meadow_02_A_M.tga", "Grass/Models/Textures/T_grass_meadow_02_N.png");
      AddAsset(GRASS_MEADOW_02_DETAILED_2, "Grass/Models/grass_meadow_02_detailed_2.fbx", "Grass/Models/Textures/T_grass_meadow_02_A_M.tga", "Grass/Models/Textures/T_grass_meadow_02_N.png");
      AddAsset(GRASS_MEADOW_03_1, "Grass/Models/grass_meadow_03_1.fbx", "Grass/Models/Textures/T_grass_meadow_03_A_M.tga");
      AddAsset(GRASS_MEADOW_03_2, "Grass/Models/grass_meadow_03_2.fbx", "Grass/Models/Textures/T_grass_meadow_03_A_M.tga");
      AddAsset(GRASS_MEADOW_03_3, "Grass/Models/grass_meadow_03_3.fbx", "Grass/Models/Textures/T_grass_meadow_03_A_M.tga");
      AddAsset(GRASS_MEADOW_03_4, "Grass/Models/grass_meadow_03_4.fbx", "Grass/Models/Textures/T_grass_meadow_03_A_M.tga");

      // Trees
      AddAsset(POPLAR_PLANT_A_00, "Trees/Models/00_Poplar_plant_A.fbx");
      AddAsset(POPLAR_PLANT_A_CROSS_00, "Trees/Models/00_Poplar_plant_A_cross.fbx", "Trees/Models/Textures/T_Poplar_00_A_Cross_A_T.png");
      AddAsset(POPLAR_PLANT_B_00, "Trees/Models/00_Poplar_plant_B.fbx");
      AddAsset(POPLAR_PLANT_B_CROSS_00, "Trees/Models/00_Poplar_plant_B_cross.fbx", "Trees/Models/Textures/T_Poplar_00_B_Cross_A_T.png");
      AddAsset(POPLAR_PLANT_C_00, "Trees/Models/00_Poplar_plant_C.fbx");
      AddAsset(POPLAR_PLANT_C_CROSS_00, "Trees/Models/00_Poplar_plant_C_cross.fbx", "Trees/Models/Textures/T_Poplar_00_C_Cross_A_T.png");
      AddAsset(POPLAR_01, "Trees/Models/01_Poplar.fbx");
      AddAsset(POPLAR_CROSS_01, "Trees/Models/01_Poplar_cross.fbx", "Trees/Models/Textures/T_Poplar_01_Cross_A_T.png");
      AddAsset(POPLAR_02, "Trees/Models/02_Poplar.fbx");
      AddAsset(POPLAR_CROSS_02, "Trees/Models/02_Poplar_cross.fbx", "Trees/Models/Textures/T_Poplar_02_Cross_A_T.png");
      AddAsset(POPLAR_CROSS_03, "Trees/Models/03_Poplar_cross.fbx", "Trees/Models/Textures/T_Poplar_03_Cross_A_T.png");
      AddAsset(POPLAR_SKAN_03, "Trees/Models/03_Poplar_skan.fbx");
      AddAsset(POPLAR_CROSS_04, "Trees/Models/04_Poplar_cross.fbx", "Trees/Models/Textures/T_Poplar_04_Cross_A_T.png");
      AddAsset(POPLAR_SKAN_04, "Trees/Models/04_Poplar_skan.fbx");
      AddAsset(POPLAR_CROSS_05, "Trees/Models/05_Poplar_cross.fbx", "Trees/Models/Textures/T_Poplar_05_Cross_A_T.png");
      //AddAsset(POPLAR_SKAN_05, "Trees/Models/05_Poplar_skan.fbx"); // Broken
      AddAsset(POPLAR_CROSS_06, "Trees/Models/06_Poplar_cross.fbx", "Trees/Models/Textures/T_Poplar_06_Cross_A_T.png");
      AddAsset(POPLAR_SKAN_06, "Trees/Models/06_Poplar_skan.fbx");
      AddAsset(POPLAR_CROSS_07, "Trees/Models/07_Poplar_cross.fbx", "Trees/Models/Textures/T_Poplar_07_Cross_A_T.png");
      AddAsset(POPLAR_SKAN_07, "Trees/Models/07_Poplar_skan.fbx");
      AddAsset(POPLAR_CROSS_08, "Trees/Models/08_Poplar_cross.fbx", "Trees/Models/Textures/T_Poplar_08_Cross_A_T.png");
      //AddAsset(POPLAR_SKAN_08, "Trees/Models/08_Poplar_skan.fbx"); // Broken

      // Rocks
      std::string rockDiffuse = "Rocks/Rocks/Models/Textures/T_Photoscanned_rocks_01_BC.tga";
      std::string rockNormal = "Rocks/Rocks/Models/Textures/T_Photoscanned_rocks_01_N.tga";
      AddAsset(M_ROCK_01, "Rocks/Rocks/Models/m_rock_01.fbx", rockDiffuse, rockNormal);
      AddAsset(M_ROCK_02, "Rocks/Rocks/Models/m_rock_02.fbx", rockDiffuse, rockNormal);
      AddAsset(M_ROCK_03, "Rocks/Rocks/Models/m_rock_03.fbx", rockDiffuse, rockNormal);
      AddAsset(ROCK_01, "Rocks/Rocks/Models/Rock_01.fbx", rockDiffuse, rockNormal);
      AddAsset(ROCK_01_CUT, "Rocks/Rocks/Models/Rock_01_cut.fbx", rockDiffuse, rockNormal);
      AddAsset(ROCK_02, "Rocks/Rocks/Models/Rock_02.fbx", rockDiffuse, rockNormal);
      AddAsset(ROCK_02_CUT, "Rocks/Rocks/Models/Rock_02_cut.fbx", rockDiffuse, rockNormal);
      AddAsset(ROCK_03, "Rocks/Rocks/Models/Rock_03.fbx", rockDiffuse, rockNormal);
      AddAsset(ROCK_03_CUT, "Rocks/Rocks/Models/Rock_03_cut.fbx", rockDiffuse, rockNormal);
      AddAsset(ROCK_04, "Rocks/Rocks/Models/Rock_04.fbx", rockDiffuse, rockNormal);
      AddAsset(ROCK_05, "Rocks/Rocks/Models/Rock_05.fbx", rockDiffuse, rockNormal);
      AddAsset(ROCK_05_CUT, "Rocks/Rocks/Models/Rock_05_cut.fbx", rockDiffuse, rockNormal);
      AddAsset(ROCK_06, "Rocks/Rocks/Models/Rock_06.fbx", rockDiffuse, rockNormal);
      AddAsset(ROCK_07, "Rocks/Rocks/Models/Rock_07.fbx", rockDiffuse, rockNormal);
      AddAsset(S_ROCK_01, "Rocks/Rocks/Models/s_rock_01.fbx", rockDiffuse, rockNormal);
      AddAsset(S_ROCK_02, "Rocks/Rocks/Models/s_rock_02.fbx", rockDiffuse, rockNormal);
      AddAsset(S_ROCK_03, "Rocks/Rocks/Models/s_rock_03.fbx", rockDiffuse, rockNormal);
      AddAsset(S_ROCK_04, "Rocks/Rocks/Models/s_rock_04.fbx", rockDiffuse, rockNormal);
      AddAsset(S_ROCK_05, "Rocks/Rocks/Models/s_rock_05.fbx", rockDiffuse, rockNormal);
      AddAsset(S_ROCK_06, "Rocks/Rocks/Models/s_rock_06.fbx", rockDiffuse, rockNormal);

      // Cliffs
      AddAsset(CLIFF_BASE_01, "Rocks/Cliffs/Models/cliff_base_01.fbx");
      AddAsset(CLIFF_BASE_02, "Rocks/Cliffs/Models/cliff_base_02.fbx");
      AddAsset(CLIFF_BASE_03, "Rocks/Cliffs/Models/cliff_base_03.fbx");
      AddAsset(CLIFF_BASE_04, "Rocks/Cliffs/Models/cliff_base_04.fbx");
      AddAsset(CLIFF_BASE_05, "Rocks/Cliffs/Models/cliff_base_05.fbx");
      AddAsset(CLIFF_BASE_06, "Rocks/Cliffs/Models/cliff_base_06.fbx");
      AddAsset(CLIFF_BASE_07, "Rocks/Cliffs/Models/cliff_base_07.fbx");
      AddAsset(CLIFF_OVERPAINT_01, "Rocks/Cliffs/Models/cliff_overpaint_01.fbx");
      AddAsset(CLIFF_OVERPAINT_02, "Rocks/Cliffs/Models/cliff_overpaint_02.fbx");
      AddAsset(CLIFF_OVERPAINT_03, "Rocks/Cliffs/Models/cliff_overpaint_03.fbx");
      AddAsset(CLIFF_OVERPAINT_04, "Rocks/Cliffs/Models/cliff_overpaint_04.fbx");
      AddAsset(CLIFF_PIECE_01, "Rocks/Cliffs/Models/cliff_piece_01.fbx");
      AddAsset(CLIFF_PIECE_02, "Rocks/Cliffs/Models/cliff_piece_02.fbx");
      AddAsset(CLIFF_PIECE_03, "Rocks/Cliffs/Models/cliff_piece_03.fbx");
      AddAsset(CLIFF_PIECE_04, "Rocks/Cliffs/Models/cliff_piece_04.fbx");
      AddAsset(CLIFF_PIECE_05, "Rocks/Cliffs/Models/cliff_piece_05.fbx");
      AddAsset(CLIFF_PIECE_06, "Rocks/Cliffs/Models/cliff_piece_06.fbx");
      AddAsset(CLIFF_PIECE_07, "Rocks/Cliffs/Models/cliff_piece_07.fbx");
      AddAsset(CLIFF_PIECE_08, "Rocks/Cliffs/Models/cliff_piece_08.fbx");
      AddAsset(CLIFF_PIECE_09, "Rocks/Cliffs/Models/cliff_piece_09.fbx");
      AddAsset(CLIFF_PIECE_10, "Rocks/Cliffs/Models/cliff_piece_10.fbx");

      // Bushes
      AddAsset(GREY_WILLOW_01, "Bushes/Models/grey_willow_01.fbx");
      AddAsset(GREY_WILLOW_01_N, "Bushes/Models/grey_willow_01_n.fbx");
      AddAsset(GREY_WILLOW_02, "Bushes/Models/grey_willow_02.fbx");
      AddAsset(GREY_WILLOW_02_CROSS, "Bushes/Models/grey_willow_02_cross.fbx", "Bushes/Models/Textures/T_Willow_Bush_02_Cross_A_T.png");
      AddAsset(GREY_WILLOW_02_N, "Bushes/Models/grey_willow_02_n.fbx");
      AddAsset(GREY_WILLOW_03, "Bushes/Models/grey_willow_03.fbx");
      AddAsset(GREY_WILLOW_03_CROSS, "Bushes/Models/grey_willow_03_cross.fbx", "Bushes/Models/Textures/T_Willow_Bush_03_Cross_A_T.png");
      AddAsset(GREY_WILLOW_03_N, "Bushes/Models/grey_willow_03_n.fbx");
      AddAsset(GREY_WILLOW_04, "Bushes/Models/grey_willow_04.fbx");
      AddAsset(GREY_WILLOW_04_CROSS, "Bushes/Models/grey_willow_04_cross.fbx", "Bushes/Models/Textures/T_Willow_Bush_04_Cross_A_T.png");
      AddAsset(GREY_WILLOW_04_N, "Bushes/Models/grey_willow_04_n.fbx");
      AddAsset(MAPLE_BUSH_01, "Bushes/Models/maple_bush_01.fbx", "Bushes/Models/Textures/T_maple_bush_BC.tga", "Bushes/Models/Textures/T_maple_bush_N.tga");
      AddAsset(MAPLE_BUSH_01_CROSS, "Bushes/Models/maple_bush_01_cross.fbx", "Bushes/Models/Textures/T_Maple_01_Cross_A_T.png");
      AddAsset(MAPLE_BUSH_02, "Bushes/Models/maple_bush_02.fbx", "Bushes/Models/Textures/T_maple_bush_BC.tga", "Bushes/Models/Textures/T_maple_bush_N.tga");
      AddAsset(MAPLE_BUSH_02_CROSS, "Bushes/Models/maple_bush_02_cross.fbx", "Bushes/Models/Textures/T_Maple_02_Cross_A_T.png");
      AddAsset(MAPLE_BUSH_03, "Bushes/Models/maple_bush_03.fbx", "Bushes/Models/Textures/T_maple_bush_BC.tga", "Bushes/Models/Textures/T_maple_bush_N.tga");
      AddAsset(MAPLE_BUSH_03_CROSS, "Bushes/Models/maple_bush_03_cross.fbx", "Bushes/Models/Textures/T_Maple_03_Cross_A_T.png");
      AddAsset(MAPLE_BUSH_04, "Bushes/Models/maple_bush_04.fbx", "Bushes/Models/Textures/T_maple_bush_BC.tga", "Bushes/Models/Textures/T_maple_bush_N.tga");
      AddAsset(MAPLE_BUSH_04_CROSS, "Bushes/Models/maple_bush_04_cross.fbx", "Bushes/Models/Textures/T_Maple_04_Cross_A_T.png");
   }

   void AssetLoader::AddAsset(uint32_t id, std::string model, std::string texture, std::string normalMap)
   {
      mAssets.push_back(Asset(id, model, texture, normalMap));
   }

   Asset AssetLoader::FindAsset(uint32_t id)
   {
      for (uint32_t i = 0; i < mAssets.size(); i++)
      {
         if (mAssets[i].id == id)
            return mAssets[i];
      }

      assert(0);
   }

   SharedPtr<Vk::StaticModel> AssetLoader::LoadAsset(uint32_t assetId)
   {
      Asset asset = FindAsset(assetId);

      std::string fullModelPath = "data/NatureManufacture/Meadow Environment Dynamic Nature/" + asset.model;

      SharedPtr<Vk::StaticModel> model = Vk::gModelLoader().LoadModel(fullModelPath);

      // Some assets are not properly storing texture paths so we need to set them manually
      if (asset.diffuseTexture != "-")
      {
         std::string fullDiffusePath = "data/NatureManufacture/Meadow Environment Dynamic Nature/" + asset.diffuseTexture;
         std::string fullNormalPath = DEFAULT_NORMAL_MAP_TEXTURE;

         if (asset.normalMap != "-")
            fullNormalPath = "data/NatureManufacture/Meadow Environment Dynamic Nature/" + asset.normalMap;

         SharedPtr<Vk::Texture> diffuseTexture = Vk::gTextureLoader().LoadTexture(fullDiffusePath);
         SharedPtr<Vk::Texture> normalMap = Vk::gTextureLoader().LoadTexture(fullDiffusePath);

         if (diffuseTexture != nullptr && normalMap != nullptr)
         {
            model->mMeshes[0]->LoadTextures(fullDiffusePath, fullNormalPath);
         }
         else
            assert(0);
      }

      return model;
   }

   Asset AssetLoader::GetAssetByIndex(uint32_t index) const
   {
      assert(index < mAssets.size());

      return mAssets[index];
   }

   uint32_t AssetLoader::GetNumAssets() const
   {
      return (uint32_t)mAssets.size();
   }
}