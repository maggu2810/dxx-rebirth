/*
 * This file is part of the DXX-Rebirth project <https://www.dxx-rebirth.com/>.
 * It is copyright by its individual contributors, as recorded in the
 * project's Git history.  See COPYING.txt at the top level for license
 * terms and a link to the Git history.
 */

/*
 *
 * Segment Loading Stuff
 *
 */


#include "segment.h"
#include "physfsx.h"
#include "physfs-serial.h"
#include "d_underlying_value.h"

namespace dcx {

namespace {

struct composite_side
{
	const shared_side &sside;
	const unique_side &uside;
};

DEFINE_SERIAL_UDT_TO_MESSAGE(composite_side, s, (s.sside.wall_num, s.uside.tmap_num, s.uside.tmap_num2));
ASSERT_SERIAL_UDT_MESSAGE_SIZE(composite_side, 6);

}

std::optional<sidenum_t> build_sidenum_from_untrusted(const uint8_t untrusted)
{
	switch (untrusted)
	{
		case static_cast<uint8_t>(sidenum_t::WLEFT):
		case static_cast<uint8_t>(sidenum_t::WTOP):
		case static_cast<uint8_t>(sidenum_t::WRIGHT):
		case static_cast<uint8_t>(sidenum_t::WBOTTOM):
		case static_cast<uint8_t>(sidenum_t::WBACK):
		case static_cast<uint8_t>(sidenum_t::WFRONT):
			return sidenum_t{untrusted};
		default:
			return std::nullopt;
	}
}

void segment_side_wall_tmap_write(PHYSFS_File *fp, const shared_side &sside, const unique_side &uside)
{
	PHYSFSX_serialize_write(fp, composite_side{sside, uside});
}

}

#if defined(DXX_BUILD_DESCENT_II)
namespace dsx {

static std::optional<delta_light_index> build_delta_light_index_from_untrusted(const uint16_t i)
{
	if (i < MAX_DELTA_LIGHTS)
		return delta_light_index{i};
	else
		return std::nullopt;
}

/*
 * reads a delta_light structure from a PHYSFS_File
 */
void delta_light_read(delta_light *dl, PHYSFS_File *fp)
{
	{
		const auto s = segnum_t{static_cast<uint16_t>(PHYSFSX_readShort(fp))};
		dl->segnum = vmsegidx_t::check_nothrow_index(s) ? s : segment_none;
	}
	dl->sidenum = build_sidenum_from_untrusted(PHYSFSX_readByte(fp)).value_or(sidenum_t::WLEFT);
	PHYSFSX_readByte(fp);
	dl->vert_light[side_relative_vertnum::_0] = PHYSFSX_readByte(fp);
	dl->vert_light[side_relative_vertnum::_1] = PHYSFSX_readByte(fp);
	dl->vert_light[side_relative_vertnum::_2] = PHYSFSX_readByte(fp);
	dl->vert_light[side_relative_vertnum::_3] = PHYSFSX_readByte(fp);
}


/*
 * reads a dl_index structure from a PHYSFS_File
 */
void dl_index_read(dl_index *di, PHYSFS_File *fp)
{
	{
		const auto s = segnum_t{static_cast<uint16_t>(PHYSFSX_readShort(fp))};
		di->segnum = vmsegidx_t::check_nothrow_index(s) ? s : segment_none;
	}
	di->sidenum = build_sidenum_from_untrusted(PHYSFSX_readByte(fp)).value_or(sidenum_t::WLEFT);
	const auto count = PHYSFSX_readByte(fp);
	if (const auto i = build_delta_light_index_from_untrusted(PHYSFSX_readShort(fp)); i)
	{
		di->count = count;
		di->index = *i;
	}
	else
	{
		di->count = 0;
		di->index = {};
	}
}

void segment2_write(const cscusegment s2, PHYSFS_File *fp)
{
	PHYSFSX_writeU8(fp, underlying_value(s2.s.special));
	PHYSFSX_writeU8(fp, underlying_value(s2.s.matcen_num));
	PHYSFSX_writeU8(fp, underlying_value(s2.s.station_idx));
	PHYSFSX_writeU8(fp, underlying_value(s2.s.s2_flags));
	PHYSFSX_writeFix(fp, s2.u.static_light);
}

void delta_light_write(const delta_light *dl, PHYSFS_File *fp)
{
	PHYSFS_writeSLE16(fp, dl->segnum);
	PHYSFSX_writeU8(fp, underlying_value(dl->sidenum));
	PHYSFSX_writeU8(fp, 0);
	PHYSFSX_writeU8(fp, dl->vert_light[side_relative_vertnum::_0]);
	PHYSFSX_writeU8(fp, dl->vert_light[side_relative_vertnum::_1]);
	PHYSFSX_writeU8(fp, dl->vert_light[side_relative_vertnum::_2]);
	PHYSFSX_writeU8(fp, dl->vert_light[side_relative_vertnum::_3]);
}

void dl_index_write(const dl_index *di, PHYSFS_File *fp)
{
	PHYSFS_writeSLE16(fp, di->segnum);
	PHYSFSX_writeU8(fp, underlying_value(di->sidenum));
	PHYSFSX_writeU8(fp, di->count);
	PHYSFS_writeSLE16(fp, underlying_value(di->index));
}

}
#endif
