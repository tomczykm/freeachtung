#include "inc/Thing.h"

Thing::Thing( float x, float y ):
  xPos_( x ),
  yPos_( y )
{

}

SDL_Rect Thing::GetRenderRect() const {
  //when w and h are 0, the texture rendered is not to be scaled
  SDL_Rect rec = { static_cast< int >( xPos_ ), static_cast< int >( yPos_ ), 0, 0 };
  return rec;
}