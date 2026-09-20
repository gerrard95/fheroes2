/***************************************************************************
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2019 - 2025                                             *
 *                                                                         *
 *   Free Heroes2 Engine: http://sourceforge.net/projects/fheroes2         *
 *   Copyright (C) 2012 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#pragma once

#include <cstdint>
#include <string>

#include "image.h"
#include "interface_itemsbar.h"
#include "math_base.h"
#include "ui_tool.h"

class Army;
class ArmyTroop;

// Width of a mini army slot that still lets all Army::maximumTroopCount slots fit into the space the dialog
// artwork reserves for the bar. Returns 'originalWidth' when there is enough room for all of them at that size.
int32_t getMiniArmySlotWidth( const int32_t availableWidth, const int32_t spacing, const int32_t originalWidth );

class ArmyBar : public Interface::ItemsActionBar<ArmyTroop>
{
public:
    using Interface::ItemsActionBar<ArmyTroop>::RedrawItem;
    using Interface::ItemsActionBar<ArmyTroop>::ActionBarRightMouseHold;

    ArmyBar( Army *, const bool miniSprites, const bool readOnly, const bool isEditMode = false, const bool saveLastTroop = true );

    void RedrawBackground( const fheroes2::Rect &, fheroes2::Image & ) override;
    void RedrawItem( ArmyTroop &, const fheroes2::Rect &, bool, fheroes2::Image & ) override;

    void SetBackground( const fheroes2::Size & sz, const uint8_t fillColor );
    void SetArmy( Army * );

    void setTroopWindowOffsetY( const int32_t offsetY )
    {
        _troopWindowOffsetY = offsetY;
    }

    bool isValid() const
    {
        return _army != nullptr;
    }

    void ResetSelected();
    void Redraw( fheroes2::Image & dstsf );

    bool ActionBarLeftMouseSingleClick( ArmyTroop & troop ) override;
    bool ActionBarLeftMouseSingleClick( ArmyTroop & destTroop, ArmyTroop & selectedTroop ) override;
    bool ActionBarLeftMouseDoubleClick( ArmyTroop & troop ) override;
    bool ActionBarLeftMouseRelease( ArmyTroop & troop ) override;
    bool ActionBarLeftMouseRelease( ArmyTroop & destTroop, ArmyTroop & troop ) override;
    bool ActionBarRightMouseHold( ArmyTroop & troop ) override;
    bool ActionBarRightMouseSingleClick( ArmyTroop & troop ) override;
    bool ActionBarRightMouseSingleClick( ArmyTroop & destTroop, ArmyTroop & selectedTroop ) override;

    bool ActionBarCursor( ArmyTroop & ) override;
    bool ActionBarCursor( ArmyTroop &, ArmyTroop & ) override;

    bool QueueEventProcessing( std::string * = nullptr );
    bool QueueEventProcessing( ArmyBar &, std::string * = nullptr );

protected:
    fheroes2::MovableSprite spcursor;

private:
    bool AbleToRedistributeArmyOnRightMouseSingleClick( const ArmyTroop & troop );

    // Draws the given full-size slot image into 'pos', scaled down and centred if the bar uses reduced slots.
    void _drawFullSizeSlot( const fheroes2::Image & slot, const fheroes2::Rect & pos, fheroes2::Image & output ) const;

    // Rectangle occupied by the slot image inside a cell of the bar. They differ only when the slots are scaled down.
    fheroes2::Rect _slotRoi( const fheroes2::Rect & pos ) const;

    Army * _army{ nullptr };
    fheroes2::Image backsf;

    // Size at which a full-size slot is drawn. Equal to the size of ICN::STRIP frame 2 unless the army has too many
    // slots to fit the bar area of the original artwork, in which case the tiles are scaled down.
    fheroes2::Size _fullSizeSlotSize;

    bool _slotsAreScaled{ false };

    bool use_mini_sprite{ false };
    bool read_only{ false };
    bool can_change{ false };
    bool _saveLastTroop{ true };
    std::string msg;
    int32_t _troopWindowOffsetY{ 0 };
};
