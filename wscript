# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('qd-channel', ['core', 'spectrum'])
    module.source = [
        'model/qd-channel-model.cc',
        ]

    module_test = bld.create_ns3_module_test_library('qd-channel')
    module_test.source = [
        'test/qd-channel-test-suite.cc',
        ]
    # Tests encapsulating example programs should be listed here
    if (bld.env['ENABLE_EXAMPLES']):
        module_test.source.extend([
        #    'test/qd-channel-examples-test-suite.cc',
             ])

    headers = bld(features='ns3header')
    headers.module = 'qd-channel'
    headers.source = [
        'model/qd-channel-model.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

