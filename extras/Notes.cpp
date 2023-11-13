#include <cstdint>
#include <array>

auto foo() -> std::array<uint32_t, 13>
{
	extern const uint32_t m_s_CRTC_latches_vtotal;
	extern const uint32_t m_s_CRTC_latches_vdisplay_end;
	extern const uint32_t m_s_CRTC_latches_start_vblank;
	extern const uint32_t m_s_CRTC_end_vblank;
	extern const uint32_t m_s_CRTC_latches_vretrace_start;
	extern const uint32_t m_s_CRTC_vretrace_end_VRE;
 	extern const uint32_t m_s_CRTC_htotal;
 	extern const uint32_t m_s_sequencer_clocking_DC;
 	extern const uint32_t m_s_CRTC_hdisplay_end;
 	extern const uint32_t m_s_CRTC_start_hblank;
 	extern const uint32_t m_s_CRTC_latches_end_hblank;
 	extern const uint32_t m_s_CRTC_start_hretrace;
 	extern const uint32_t m_s_CRTC_end_hretrace_EHR;
 	extern const uint32_t m_s_sequencer_clocking_D89;

	const uint32_t vtotal  =  m_s_CRTC_latches_vtotal + 2;
	const uint32_t vdend   =  m_s_CRTC_latches_vdisplay_end + 1;
	uint32_t vbstart =  m_s_CRTC_latches_start_vblank;
	uint32_t vbend   =  m_s_CRTC_end_vblank & 0x7f;
	const uint32_t vrstart =  m_s_CRTC_latches_vretrace_start;
	uint32_t vrend   = (m_s_CRTC_vretrace_end_VRE - vrstart) & 0xF;

	if(vrend == 0) {
		vrend = vrstart + 0xf + 1;
	} else {
		vrend = vrstart + vrend;
	}

	if(vbstart != 0) {
		vbstart += 1;
		vbend = (vbend - vbstart) & 0x7f;
		if(vbend == 0) {
			vbend = vbstart + 0x7f + 1;
		} else {
			vbend = vbstart + vbend;
		}
	} else {
		// When vbstart is 0, lines zero to vbend are blanked.
		// According to DosBox:
		//   ET3000 blanks lines 1 to vbend (255/6 lines).
		//   ET4000 doesn't blank if vbstart == vbend.
	}
	vbend++;

	// HORIZONTAL TIMINGS

	const uint32_t htotal  = (m_s_CRTC_htotal + 5) << m_s_sequencer_clocking_DC;
	const uint32_t hdend   =  m_s_CRTC_hdisplay_end + 1;
	const uint32_t hbstart =  m_s_CRTC_start_hblank;
	const uint32_t hbend   =  hbstart + ((m_s_CRTC_latches_end_hblank - hbstart) & 0x3F);
	const uint32_t hrstart =  m_s_CRTC_start_hretrace;
	uint32_t hrend   = (m_s_CRTC_end_hretrace_EHR - hrstart) & 0x1F;
	const uint32_t cwidth  =  m_s_sequencer_clocking_D89 ? 8 : 9;

	if(hrend == 0) {
		hrend = hrstart + 0x1f + 1;
	} else {
		hrend = hrstart + hrend;
	}

    extern volatile uint32_t G_vtotal ;
    extern volatile uint32_t G_vdend  ;
    extern volatile uint32_t G_vbstart;
    extern volatile uint32_t G_vbend  ;
    extern volatile uint32_t G_vrstart;
    extern volatile uint32_t G_vrend  ;
    extern volatile uint32_t G_htotal ;
    extern volatile uint32_t G_hdend  ;
    extern volatile uint32_t G_hbstart;
    extern volatile uint32_t G_hbend  ;
    extern volatile uint32_t G_hrstart;
    extern volatile uint32_t G_hrend  ;
    extern volatile uint32_t G_cwidth ;

	G_vtotal 	= vtotal  ;
	G_vdend  	= vdend   ;
	G_vbstart	= vbstart ;
	G_vbend  	= vbend   ;
	G_vrstart	= vrstart ;
	G_vrend  	= vrend   ;
	G_htotal 	= htotal  ;
	G_hdend  	= hdend   ;
	G_hbstart	= hbstart ;
	G_hbend  	= hbend   ;
	G_hrstart	= hrstart ;
	G_hrend  	= hrend   ;
	G_cwidth 	= cwidth  ;
}