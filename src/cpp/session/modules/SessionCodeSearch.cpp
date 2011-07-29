/*
 * SessionCodeSearch.cpp
 *
 * Copyright (C) 2009-11 by RStudio, Inc.
 *
 * This program is licensed to you under the terms of version 3 of the
 * GNU Affero General Public License. This program is distributed WITHOUT
 * ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
 * AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
 *
 */

#include "SessionCodeSearch.hpp"

#include <vector>

#include <boost/bind.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <core/Error.hpp>
#include <core/Exec.hpp>

#include <session/SessionUserSettings.hpp>
#include <session/SessionModuleContext.hpp>

#include <session/SessionSourceDatabase.hpp>
#include <session/projects/SessionProjects.hpp>

using namespace core ;

namespace session {  
namespace modules {
namespace code_search {

namespace {
   
// TODO: rpc array result type (wire efficiency)

json::Object codeSearchResult(const r_util::RFunctionInfo& functionInfo)
{
   json::Object result;
   result["name"] = functionInfo.name();
   result["context"] = functionInfo.context();
   try
   {
      result["line"] = boost::numeric_cast<int>(functionInfo.line());
      result["column"] = boost::numeric_cast<int>(functionInfo.column());
   }
   catch (const boost::bad_numeric_cast& e)
   {
      result["line"] = 1;
      result["column"] = 1;
   }

   return result;
}

Error searchCode(const json::JsonRpcRequest& request,
                 json::JsonRpcResponse* pResponse)
{
   std::string term;
   Error error = json::readParam(request.params, 0, &term);
   if (error)
      return error;

   std::vector<r_util::RFunctionInfo> functions;
   projects::projectContext().findFunctions(term,
                                            true,
                                            std::back_inserter(functions));

   json::Array results;
   std::transform(functions.begin(),
                  functions.end(),
                  std::back_inserter(results),
                  codeSearchResult);

   pResponse->setResult(results);

   return Success();
}


   
} // anonymous namespace


   
Error initialize()
{
   using boost::bind;
   using namespace module_context;
   ExecBlock initBlock ;
   initBlock.addFunctions()
      (bind(registerRpcMethod, "search_code", searchCode));
   ;

   return initBlock.execute();
}


} // namespace agreement
} // namespace modules
} // namespace session