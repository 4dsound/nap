#pragma once

#include <utility/stringutils.h>

#include <vector>
#include <string>

namespace nap
{
	namespace utility
	{
		/**
		 * ErrorState is a class that can be used to maintain an error state over (nested) function calls. It's useful to be able to return detailed error information
		 * from somewhere deep down the stack. It's kind of a poor-man's exception handling in that regard.
		 */
		class ErrorState
		{
		public:
			/**
			 * Check whether the specified condition evaluates to true and adds an error message to the state if not
			 *
			 * @param successCondition The condition to check (i.e. true/false)
			 * @param errorString The error message that belongs to the 'fail' state
			 * @return Whether the condition evaluated to true (i.e. success) or not
			 */
			bool check(bool successCondition, const std::string& errorString)
			{
				if (!successCondition)
					mErrorList.push_back(errorString);

				return successCondition;
			}

			/**
			 * Same as non-templated check(), but here to allow for easy formatting of error messages
			 */
			template <typename... Args>
			bool check(bool successCondition, const std::string& format, Args&&... args)
			{
				return check(successCondition, stringFormat(format, std::forward<Args>(args)...));
			}

			/**
			 * Added a failure message to the stack. Useful in situations where you already know that you've failed, in which case the check() function is redundant.
			 *
			 * @param errorString The error message that belongs to the 'fail' state
			 */
			void fail(const std::string& errorString)
			{
				mErrorList.push_back(errorString);
			}

			/**
			 * Same as non-templated fail(), but here to allow for easy formatting of error messages
			 */
			template <typename... Args>
			void fail(const std::string& format, Args&&... args)
			{
				fail(stringFormat(format, std::forward<Args>(args)...));
			}

			/**
			 * Format the error state of this object into a human-readable message
			 *
			 * @return The full error message
			 */
			const std::string toString() const;

		private:
			std::vector<std::string> mErrorList;
		};

	}
}
