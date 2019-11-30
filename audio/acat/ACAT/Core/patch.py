############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2018 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries.  All rights reserved.
#
############################################################################
"""
Provides patching for processors
"""
import logging
import os

from ACAT.Core.KerDebugInfo import KerDebugInfo

try:
    # Import the optional external library
    from ACAT.Core.BuildFinder import BuildFinder
except ImportError:
    BuildFinder = None

logger = logging.getLogger(__name__)


class ProcessorPatcher(object):
    """Patches a Processor instance.

    Args:
        processor: A Processor instance.
        patch_path (str, optional): Path to a patch.
    """

    def __init__(self, processor, patch_path=None):
        self._processor = processor
        self._patch_path = patch_path

        # Read the build's patch id from the processor
        self._build_patch_id = processor.get_patch_id()
        self._build_patch_path = None

    def apply(self):
        """Checks the conditions and patch the processor.

        Here is a brief explanation on conditions:

        - When the build's patch ID is zero but there is a patch provided:
            Logs a warning message and will not patch the processor.

        - When the build's patch ID is non-zero but no patch is provided:
            Try to get the build's patch and apply it on the processor.

        - When the build's patch ID is non-zero and a patch is provided too:
            Applies the patch.

        - When the build's patch ID is zero and no patch is provided:
            Do nothing.
        """
        if self._build_patch_id == 0 and self._patch_path is not None:
            logger.warning(
                "The build is not associated with a patch but extra patch(es) "
                "provided."
            )

        elif self._build_patch_id and self._patch_path is None:
            # Build is associated with a patch but there is no patch
            # given. Try to load the appropriate patch.
            logger.debug(
                "Processor's build needs a patch but no patch is provided. "
                "Trying to apply the patch ID: %s.", self._build_patch_id
            )
            try:
                self._find_build_patch()
                self._patch()
                logger.debug("Patched with build ID: %s", self._build_patch_id)

            except ImportError:
                # Build Finder is missing. Nothing will be loaded.
                logger.warning(
                    "The build is associated with a patch but no patch "
                    "is provided"
                )
            except Exception as error:
                logger.warning(
                    'The build is patched with an invalid ID. %s',
                    error
                )

        elif self._build_patch_id and self._patch_path:
            # Patches don't have build identifiers. So we can't determine
            # whether the provided patch is the same as the required one
            # from the build. See B-259907.
            self._patch()
            logger.debug("Provided patch is applied.")

        else:
            logger.debug(
                "Build's patch ID is zero and no patch is provided. No need "
                "to patch the Processor."
            )

    def _find_build_patch(self):
        """Uses an external library to find the appropriate patch.

        We don't know what exception the external optional library is going
        to raise, that's why a generic exception trap should catch it.

        Raises:
            ImportError: When the external library is invalid.
            Exception: When the provided patch_id is invalid.
        """
        if BuildFinder is None:
            raise ImportError('BuildFinder not found')

        # Build Finder's path to the patch might be `None` if it's not found
        build_finder = BuildFinder(self._build_patch_id)
        self._build_patch_path = build_finder.patch_build_path

    def _patch(self):
        """ Patches the processor.

        If there is a patch given, it will use that, otherwise it will use
        the build's patch path.
        """
        if self._patch_path is None:
            patch_path = self._build_patch_path
        else:
            patch_path = self._patch_path

        if patch_path is not None:
            if not patch_path.endswith(('.elf', '.zip', 'kymera-patch')):
                # The path doesn't have the filename, so adding it
                patch_path = os.path.join(patch_path, 'kymera-patch.elf')

            debug_info = self._get_debug_info(patch_path)
            self._processor.debuginfo.update_patches(
                {
                    debug_info.elf_id: debug_info
                }
            )

        else:
            logger.warning("Coudn't find the right patch!")

    @staticmethod
    def _get_debug_info(build_path):
        """Instantiates KerDebugInfo object with the given build path.

        Args:
            build_path (str): A valid path to a build.

        Returns:
            An instance of KerDebugInfo object.
        """
        debug_info = KerDebugInfo(build_path)
        debug_info.read_debuginfo()

        return debug_info
