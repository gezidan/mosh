/*
    Mosh: the mobile shell
    Copyright 2012 Keith Winstein

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    In addition, as a special exception, the copyright holders give
    permission to link the code of portions of this program with the
    OpenSSL library under certain conditions as described in each
    individual source file, and distribute linked combinations including
    the two.

    You must obey the GNU General Public License in all respects for all
    of the code used other than OpenSSL. If you modify file(s) with this
    exception, you may extend this exception to your version of the
    file(s), but you are not obligated to do so. If you do not wish to do
    so, delete this exception statement from your version. If you delete
    this exception statement from all source files in the program, then
    also delete it here.
*/

#ifndef STM_CLIENT_HPP
#define STM_CLIENT_HPP

#include <sys/ioctl.h>
#include <termios.h>
#include <string>

#include "completeterminal.h"
#include "networktransport.h"
#include "user.h"
#include "terminaloverlay.h"

class STMClient {
private:
  std::string ip;
  int port;
  std::string key;

  struct termios saved_termios, raw_termios;

  struct winsize window_size;

  Terminal::Framebuffer *local_framebuffer, *new_state;
  Overlay::OverlayManager overlays;
  Network::Transport< Network::UserStream, Terminal::Complete > *network;
  Terminal::Display display;

  std::string connecting_notification;
  bool repaint_requested, quit_sequence_started;
  bool clean_shutdown;

  void main_init( void );
  bool process_network_input( void );
  bool process_user_input( int fd );
  bool process_resize( void );

  void output_new_frame( void );

  bool still_connecting( void )
  {
    /* Initially, network == NULL */
    return network && ( network->get_remote_state_num() == 0 );
  }

public:
  STMClient( const char *s_ip, int s_port, const char *s_key, const char *predict_mode )
    : ip( s_ip ), port( s_port ), key( s_key ),
      saved_termios(), raw_termios(),
      window_size(),
      local_framebuffer( NULL ),
      new_state( NULL ),
      overlays(),
      network( NULL ),
      display( true ), /* use TERM environment var to initialize display */
      connecting_notification(),
      repaint_requested( false ),
      quit_sequence_started( false ),
      clean_shutdown( false )
  {
    if ( predict_mode ) {
      if ( !strcmp( predict_mode, "always" ) ) {
	overlays.get_prediction_engine().set_display_preference( Overlay::PredictionEngine::Always );
      } else if ( !strcmp( predict_mode, "never" ) ) {
	overlays.get_prediction_engine().set_display_preference( Overlay::PredictionEngine::Never );
      } else if ( !strcmp( predict_mode, "adaptive" ) ) {
	overlays.get_prediction_engine().set_display_preference( Overlay::PredictionEngine::Adaptive );
      } else if ( !strcmp( predict_mode, "experimental" ) ) {
	overlays.get_prediction_engine().set_display_preference( Overlay::PredictionEngine::Experimental );
      } else {
	fprintf( stderr, "Unknown prediction mode %s.\n", predict_mode );
	exit( 1 );
      }
    }
  }

  void init( void );
  void shutdown( void );
  void main( void );

  ~STMClient()
  {
    if ( local_framebuffer != NULL ) {
      delete local_framebuffer;
    }

    if ( new_state != NULL ) {
      delete new_state;
    }

    if ( network != NULL ) {
      delete network;
    }
  }

  /* unused */
  STMClient( const STMClient & );
  STMClient & operator=( const STMClient & );
};

#endif
