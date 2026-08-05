"""test_09_mode_manager.py:

Rudimentary integration tests for the deployment's ModePolicy transition
gates on top of Svc::ModeManager. Deliberately narrow: only the transitions
ModePolicy actually blocks or gates are exercised here, not a full matrix
of the (permissive) transitions. See
FprimeSoakTestReference/Components/ModePolicy/ModePolicy.fpp for the full
policy (REQ-MPOL-001 through REQ-MPOL-010).

These tests are portable across deployments: fprime_test_api.get_mnemonic()
resolves component instance names via this deployment's own int_config.json
rather than hardcoding them.
"""

from fprime_gds.common.utils.event_severity import EventSeverity


def _mode_manager(api, name):
    return f"{api.get_mnemonic('Svc.ModeManager')}.{name}"


def _mode_policy(api, name):
    return f"{api.get_mnemonic('Components.ModePolicy')}.{name}"


def _sensor_data_producer(api, name):
    return f"{api.get_mnemonic('Components.SensorDataProducer')}.{name}"


def _commander(api):
    # send_and_assert_command defaults commander="cmdDisp", which only matches
    # deployments where the command dispatcher instance is literally named
    # "cmdDisp". This deployment's is "CdhCore.cmdDisp".
    return api.get_mnemonic("Svc.CommandDispatcher")


def test_safe_to_experimentation_rejected(fprime_test_api):
    """SAFE -> EXPERIMENTATION is blocked by policy; SAFE only opens to IDLE.

    Covers: REQ-MPOL-002
    """
    commander = _commander(fprime_test_api)
    fprime_test_api.send_and_assert_command(
        _mode_manager(fprime_test_api, "START"), [], max_delay=5, commander=commander
    )
    fprime_test_api.send_and_assert_command(
        _mode_manager(fprime_test_api, "REQUEST_MODE"), ["SAFE"], max_delay=5, commander=commander
    )

    fprime_test_api.send_and_assert_command(
        _mode_manager(fprime_test_api, "REQUEST_MODE"), ["EXPERIMENTATION"], max_delay=5, commander=commander
    )

    fprime_test_api.assert_event(
        _mode_manager(fprime_test_api, "TransitionRejected"),
        ["SAFE", "EXPERIMENTATION", "GROUND", None, "EXTERNAL"],
        severity=EventSeverity.WARNING_LO,
        timeout=5,
    )
    fprime_test_api.assert_event(
        _mode_policy(fprime_test_api, "TransitionBlocked"),
        severity=EventSeverity.WARNING_LO,
        timeout=5,
    )
    fprime_test_api.assert_telemetry(
        _mode_manager(fprime_test_api, "CurrentMode"), "SAFE", timeout=5
    )


def test_experimentation_to_idle_rejected_while_serializing(fprime_test_api):
    """EXPERIMENTATION -> IDLE is blocked while SensorDataProducer is serializing.

    Covers: REQ-MPOL-007, REQ-MPOL-008
    """
    commander = _commander(fprime_test_api)
    fprime_test_api.send_and_assert_command(
        _mode_manager(fprime_test_api, "REQUEST_MODE"), ["IDLE"], max_delay=5, commander=commander
    )
    fprime_test_api.send_and_assert_command(
        _mode_manager(fprime_test_api, "REQUEST_MODE"), ["EXPERIMENTATION"], max_delay=5, commander=commander
    )
    fprime_test_api.send_and_assert_command(
        _sensor_data_producer(fprime_test_api, "START_SERIALIZING"), [], max_delay=5, commander=commander
    )

    fprime_test_api.send_and_assert_command(
        _mode_manager(fprime_test_api, "REQUEST_MODE"), ["IDLE"], max_delay=5, commander=commander
    )

    fprime_test_api.assert_event(
        _mode_manager(fprime_test_api, "TransitionRejected"),
        ["EXPERIMENTATION", "IDLE", "GROUND", None, "EXTERNAL"],
        severity=EventSeverity.WARNING_LO,
        timeout=5,
    )
    fprime_test_api.assert_telemetry(
        _mode_manager(fprime_test_api, "CurrentMode"), "EXPERIMENTATION", timeout=5
    )

    # Leave the component idle for later tests.
    fprime_test_api.send_and_assert_command(
        _sensor_data_producer(fprime_test_api, "STOP_SERIALIZING"), [], max_delay=5, commander=commander
    )


def test_experimentation_to_idle_allowed_when_not_serializing(fprime_test_api):
    """EXPERIMENTATION -> IDLE succeeds once serialization has stopped.

    Covers: REQ-MPOL-009
    """
    commander = _commander(fprime_test_api)
    fprime_test_api.send_and_assert_command(
        _mode_manager(fprime_test_api, "REQUEST_MODE"), ["IDLE"], max_delay=5, commander=commander
    )
    fprime_test_api.send_and_assert_command(
        _mode_manager(fprime_test_api, "REQUEST_MODE"), ["EXPERIMENTATION"], max_delay=5, commander=commander
    )

    fprime_test_api.send_and_assert_command(
        _mode_manager(fprime_test_api, "REQUEST_MODE"), ["IDLE"], max_delay=5, commander=commander
    )

    fprime_test_api.assert_event(
        _mode_manager(fprime_test_api, "ModeTransitioned"),
        ["EXPERIMENTATION", "IDLE", "GROUND", None],
        severity=EventSeverity.ACTIVITY_HI,
        timeout=5,
    )
    fprime_test_api.assert_telemetry(
        _mode_manager(fprime_test_api, "CurrentMode"), "IDLE", timeout=5
    )
