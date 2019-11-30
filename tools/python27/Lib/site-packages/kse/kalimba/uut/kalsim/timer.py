'''
Kalsim timers abstraction layer
'''

import logging
import threading
from functools import partial

from kats.framework.library.log import log_input, log_output, log_exception

#: constant indicating the minimum cookie to use for kalcmd timers
COOKIE_MIN = 1

#: constant indicating the maximum cookie to use for kalcmd timers
COOKIE_MAX = 10000


class Timer(object):
    '''
    Module to handle kalcmd/kalsim simulated timers

    It requires a kalcmd instance to operate.
    kalcmd timers generate a callback when expire in kalcmd background thread,
    this module attaches to the callback and keeps a list a currently running timers, both absolute
    and relative.

    The user api to this module enables to create timers and check if those timers have expired

    Args:
        kalcmd (kats.instrument.Kalcmd.Kalcmd): Kalcmd instrument
    '''

    def __init__(self, kalcmd):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._log.info('init kalcmd:%s', kalcmd)

        self._kalcmd = kalcmd
        self._kalcmd.install_timer_handler(self._timer_event)
        self._timers = {}
        self._last_cookie = COOKIE_MIN

    @log_exception
    @log_input(logging.INFO)
    def _timer_event(self, cookie, cur_time):
        self._log.info('kalsim timer cookie:%s cur_time:%s fired', cookie, cur_time)
        if cookie in self._timers:
            if self._timers[cookie]['func']:
                self._timers[cookie]['func'](cookie)
            del self._timers[cookie]

    @log_output(logging.INFO)
    def get_time(self):
        '''
        Get current time.

        Returns:
            float: Current simulation time in seconds
        '''
        return self._kalcmd.get_current_time()

    @log_input(logging.INFO)
    def add_relative_timer(self, period, callback=None):
        '''
        Add a simulated relative timer.

        To check for timer expiration use check_timer_expired or provide a callback func
        The callback should be in the format

        .. code-block:: python

            def timer_fired(timer_id):
                pass

        Note that the callback is running in the context of kalcmd background thread so it should
        not block

        Example code

        .. code-block:: python

            wait = True

            def cb(id):
                print('timer expired')
                global wait
                wait = False

            id = timer.add_relative_timer(1.0, cb)
            while wait:
                import time
                time.sleep(0.1)
            print('execution resumes')

        Args:
            period (float): Period in seconds
            callback (func): Callback function to be called when timer expires

        Returns:
            int: Timer id
        '''
        last_cookie = self._last_cookie
        while True:
            if self._last_cookie not in self._timers:
                self._timers[self._last_cookie] = {'func': callback}
                self._kalcmd.set_relative_timer(self._last_cookie, period)
                cookie = self._last_cookie
                self._last_cookie = COOKIE_MIN if self._last_cookie == COOKIE_MAX else \
                    self._last_cookie + 1
                break
            self._last_cookie += 1
            if self._last_cookie > COOKIE_MAX:
                self._last_cookie = COOKIE_MIN
            if self._last_cookie == last_cookie:
                raise RuntimeError('no available timers')

        return cookie

    @log_input(logging.INFO)
    def add_absolute_timer(self, period, callback=None):
        '''
        Add a simulated absolute timer

        To check for timer expiration use check_timer_expired or provide a callback func

        The callback should be in the format

        .. code-block:: python

            def timer_fired(timer_id):
                pass

        Note that the callback is running in the context of kalcmd background thread so it should
        not block

        Args:
            period (float): Period in seconds
            callback (func): Callback function to be called when timer expires

        Returns:
            int: Timer id
        '''
        last_cookie = self._last_cookie
        while True:
            if self._last_cookie not in self._timers:
                self._timers[self._last_cookie] = {'func': callback}
                self._kalcmd.set_absolute_timer(self._last_cookie, period)
                cookie = self._last_cookie
                self._last_cookie = COOKIE_MIN if self._last_cookie == COOKIE_MAX else \
                    self._last_cookie + 1
                break
            self._last_cookie += 1
            if self._last_cookie > COOKIE_MAX:
                self._last_cookie = COOKIE_MIN
            if self._last_cookie == last_cookie:
                raise RuntimeError('no available timers')

        return cookie

    @log_input(logging.INFO)
    def cancel_timer(self, timer_id):
        '''
        Cancel a timer

        Args:
            period (float): Period in seconds

        Returns:
            bool: True if there was a timer and it has been cancelled
        '''
        ret = False
        if timer_id in self._timers:
            # just remove it from our list of timers as there is no way in kalcmd to remove an
            # existing timer, it will get fired eventually and no action will take place
            del self._timers[timer_id]
            ret = True
        return ret

    @log_output(logging.DEBUG)
    def check_timer_expired(self, timer_id):
        '''
        Check if a timer has expired

        Args:
            period (float): Period in seconds

        Returns:
            bool: True if there is not a pending timer with the supplied id
        '''
        return not timer_id in self._timers

    @log_input(logging.INFO)
    def wait_relative_timer(self, period):
        '''
        Wait for a relative amount of time.

        Args:
            period (float): Period in seconds
        '''

        def callback(ev, timer_id):
            _ = timer_id
            ev.set()

        ev = threading.Event()
        last_cookie = self._last_cookie
        while True:
            if self._last_cookie not in self._timers:
                self._timers[self._last_cookie] = {'func': partial(callback, ev)}
                self._kalcmd.set_relative_timer(self._last_cookie, period)
                self._last_cookie = COOKIE_MIN if self._last_cookie == COOKIE_MAX else \
                    self._last_cookie + 1
                break
            self._last_cookie += 1
            if self._last_cookie > COOKIE_MAX:
                self._last_cookie = COOKIE_MIN
            if self._last_cookie == last_cookie:
                raise RuntimeError('no available timers')

        ev.wait()
