// Run with crossbowman-test.vcxproj after building the game in Release-SDL2 x64.
#include <cassert>
#include <cstring>
#include <iostream>

#include "agg.h"
#include "bin_info.h"
#include "castle.h"
#include "game_assets.h"
#include "icn.h"
#include "image.h"
#include "maps_tiles.h"
#include "maps_tiles_helper.h"
#include "monster.h"
#include "mp2.h"
#include "profit.h"
#include "race.h"
#include "settings.h"

int main( int argc, char ** argv )
{
    Settings::Get().SetProgramPath( argv[0] );
    const AGG::AGGInitializer resources;
    const Monster unit( Race::KNGT, DWELLING_MONSTER1 );
    assert( unit.GetID() == Monster::PEASANT );
    assert( unit.GetAttack() == 7 && unit.GetDefense() == 7 );
    assert( unit.GetDamageMin() == 4 && unit.GetDamageMax() == 8 );
    assert( unit.GetHitPoints() == 20 && unit.GetSpeed() == 4 );
    assert( unit.GetShots() == 12 && unit.isArchers() );
    assert( unit.GetGrown() == 5 && unit.GetCost().gold == 250 );
    assert( unit.GetDwelling() == DWELLING_MONSTER1 && !unit.isAllowUpgrade() );
    // The Knight's Farm gives gold instead of growth; other races keep their +8.
    assert( Castle::GetGrownWel2( Race::KNGT ) == 0 && Castle::GetGrownWel2( Race::BARB ) == 8 );
    assert( ProfitConditions::FromBuilding( BUILD_WEL2, Race::KNGT ).gold == 200 );
    assert( ProfitConditions::FromBuilding( BUILD_WEL2, Race::BARB ).gold == 0 );

    // Crossbowman Hut (Peasant Hut slot): free Crossbowmen, 3-8 at start and 1-2 more every week.
    assert( std::strcmp( MP2::StringObject( MP2::OBJ_PEASANT_HUT ), "Crossbowman Hut" ) == 0 );
    Maps::Tile hut;
    hut.setMainObjectType( MP2::OBJ_PEASANT_HUT );
    assert( Maps::getMonsterFromTile( hut ).GetID() == Monster::PEASANT );
    for ( int attempt = 0; attempt < 200; ++attempt ) {
        Maps::updateDwellingPopulationOnTile( hut, true );
        const uint32_t initial = Maps::getMonsterCountFromTile( hut );
        assert( initial >= 3 && initial <= 8 );
        Maps::updateDwellingPopulationOnTile( hut, false );
        const uint32_t weekly = Maps::getMonsterCountFromTile( hut ) - initial;
        assert( weekly >= 1 && weekly <= 2 );
    }
    assert( unit.GetRandomUnitLevel() == Monster::LevelType::LEVEL_3 );
    assert( Monster::GetMissileICN( unit.GetID() ) == ICN::ARCH_MSL );

    const auto & data = fheroes2::getMonsterData( unit.GetID() );
    const auto & archer = fheroes2::getMonsterData( Monster::ARCHER );
    assert( data.battleStats.abilities.empty() );
    assert( std::strcmp( data.generalStats.untranslatedName, "Crossbowman" ) == 0 );
    assert( data.sounds.meleeAttack == archer.sounds.meleeAttack );
    assert( data.sounds.rangeAttack == archer.sounds.rangeAttack );
    assert( data.sounds.movement == archer.sounds.movement );
    assert( data.sounds.wince == archer.sounds.wince );
    assert( data.sounds.death == archer.sounds.death );

    const auto animation = Bin_Info::GetMonsterInfo( unit.GetID() );
    const auto archerAnimation = Bin_Info::GetMonsterInfo( Monster::ARCHER );
    assert( animation.isValid() );
    assert( animation.frameXOffset == archerAnimation.frameXOffset );
    const bool expectCustom = argc < 2 || std::strcmp( argv[1], "--fallback" ) != 0;
    const uint32_t frameCount = Assets::getImageCount( ICN::PEASANT );
    assert( frameCount == ( expectCustom ? 49U : 51U ) );
    if ( expectCustom ) {
        assert( animation.animationFrames != archerAnimation.animationFrames );
        assert( animation.animationFrames[Bin_Info::MonsterAnimInfo::DEATH].back() == 48 );
        assert( Assets::getImage( ICN::PEASANT, 48 ).height() < Assets::getImage( ICN::PEASANT, 1 ).height() );
        // Battle walking advances by the sprite's own x offset (as in Archer frames); in-place frames would teleport.
        assert( Assets::getImage( ICN::PEASANT, 16 ).x() - Assets::getImage( ICN::PEASANT, 9 ).x() >= 30 );
    }
    else {
        assert( animation.animationFrames == archerAnimation.animationFrames );
        assert( animation.projectileOffset == archerAnimation.projectileOffset );
    }
    for ( const auto & frames : animation.animationFrames ) {
        for ( const int frame : frames ) {
            assert( frame > 0 && static_cast<uint32_t>( frame ) < frameCount );
            assert( !Assets::getImage( ICN::PEASANT, frame ).empty() );
        }
    }
    for ( const auto sequence : { Bin_Info::MonsterAnimInfo::MOVE_MAIN, Bin_Info::MonsterAnimInfo::DEATH,
                                 Bin_Info::MonsterAnimInfo::SHOOT1, Bin_Info::MonsterAnimInfo::SHOOT2,
                                 Bin_Info::MonsterAnimInfo::SHOOT3 } ) {
        assert( animation.hasAnim( sequence ) );
    }
    assert( !AGG::getDataFromAggFile( "PEASANT.ICN", false ).empty() );
    assert( !AGG::getDataFromAggFile( "MONH0000.ICN", false ).empty() );
    assert( Monster( Monster::ARCHER ).GetAttack() == 5 );
    assert( Monster( Monster::ARCHER ).GetHitPoints() == 10 );
    assert( Monster( Monster::RANGER ).isAbilityPresent( fheroes2::MonsterAbilityType::DOUBLE_SHOOTING ) );
    assert( !Assets::getImage( ICN::MONH0000, 0 ).empty() );
    assert( !Assets::getImage( ICN::MONS32, 0 ).empty() );
    for ( uint32_t frame = 0; frame < 9; ++frame ) {
        assert( !Assets::getImage( ICN::MINI_MONSTER_IMAGE, frame ).empty() );
    }
    std::cout << "PASS: Crossbowman stats, sounds, all animation frames, portrait and miniatures ("
              << ( expectCustom ? "custom blue pack" : "Archer fallback" ) << ")\n";
}
