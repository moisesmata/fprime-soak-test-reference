// ======================================================================
// \title  ModePolicy.hpp
// \brief  hpp file for ModePolicy component implementation class
// ======================================================================

#ifndef Components_ModePolicy_HPP
#define Components_ModePolicy_HPP

#include "FprimeSoakTestReference/Components/ModePolicy/ModePolicyComponentAc.hpp"

namespace Components {

  class ModePolicy :
    public ModePolicyComponentBase
  {

    public:

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct ModePolicy object
      ModePolicy(
          const char* const compName //!< The component name
      );

      //! Destroy ModePolicy object
      ~ModePolicy();

    private:

      // ----------------------------------------------------------------------
      // Handler implementations for user-defined typed input ports
      // ----------------------------------------------------------------------

      //! Handler implementation for checkTransition
      //!
      //! Check whether a mode transition is permitted
      Fw::Success checkTransition_handler(
          FwIndexType portNum, //!< The port number
          const Svc::Mode& current, //!< The current mode
          const Svc::Mode& target, //!< The requested mode
          const Svc::ModeRequest& req //!< Who requested it
      ) override;

  };

}

#endif
