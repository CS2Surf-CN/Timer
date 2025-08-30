module;

#include <core/core.h>

module surf.core.forwards;

template<>
CCoreForward* CBaseForward<CCoreForward>::m_pFirst = nullptr;
;
FORWARD_INIT(CFeatureForward);
